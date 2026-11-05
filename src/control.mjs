import http from 'node:http';
import { setupPage } from './setup-page.mjs';
import { randomBytes, timingSafeEqual } from 'node:crypto';

export async function startControlServer({ command, snapshot, setupConfig, port = 0 }) {
  const token = randomBytes(32).toString('hex'); let queue = Promise.resolve();
  const server = http.createServer(async (request, response) => {
    const origin = `http://127.0.0.1:${server.address().port}`;
    const headers = { 'cache-control': 'no-store', 'x-frame-options': 'DENY', 'referrer-policy': 'no-referrer',
      'content-security-policy': "default-src 'none'; connect-src 'self'; script-src 'unsafe-inline'; style-src 'unsafe-inline'; frame-ancestors 'none'" };
    const reply = (status, data, type = 'application/json') => { response.writeHead(status, { ...headers, 'content-type': `${type}; charset=utf-8` }); response.end(type === 'application/json' ? JSON.stringify(data) : data); };
    if (request.headers.host !== `127.0.0.1:${server.address().port}`) { reply(403, { error: 'HOST_REFUSED' }); return; }
    if (request.method === 'GET' && request.url === '/') { reply(200, page(token,!!snapshot().device), 'text/html'); return; }
    if (request.method === 'GET' && request.url === '/setup') { reply(200, setupPage(token), 'text/html'); return; }
    if (request.method === 'GET' && request.url === '/state') { reply(200, snapshot()); return; }
    if (request.method !== 'POST' || !['/command','/setup-config'].includes(request.url)) { reply(404, {}); return; }
    const supplied = Buffer.from(request.headers['x-bridge-control'] ?? '');
    if (request.headers.origin !== origin || supplied.length !== token.length || !timingSafeEqual(supplied, Buffer.from(token))) { reply(403, { error: 'CONTROL_REFUSED' }); return; }
    try {
      if (request.url === '/setup-config') { if (!setupConfig) {reply(404, {});return;} reply(200,await setupConfig());return; }
      let body = '';
      for await (const chunk of request) { body += chunk; if (body.length > 4096) throw new Error(); }
      const input = JSON.parse(body);
      if (!['connect', 'press', 'release', 'hangup', 'select'].includes(input.action)) throw new Error();
      const result = queue.then(() => command(input.action, input)); queue = result.catch(() => {});
      await result; reply(200, { ok: true });
    } catch { reply(400, { error: 'COMMAND_FAILED' }); }
  });
  server.requestTimeout = 65000; server.headersTimeout = 10000;
  await new Promise((resolve, reject) => { server.once('error', reject); server.listen(port, '127.0.0.1', resolve); });
  return { port: server.address().port, close: () => new Promise(resolve => server.close(resolve)) };
}

function page(token, device = false) {
  return `<!doctype html><html lang="es"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>Grok Orb · Puente de voz</title>
<style>*{box-sizing:border-box}body{margin:0;min-height:100vh;display:grid;place-items:center;background:#10131b;color:#f1f1f7;font:16px -apple-system,BlinkMacSystemFont,sans-serif}main{width:min(520px,92vw);padding:38px;border:1px solid #333846;border-radius:30px;background:#191e2a;text-align:center}small{color:#a4adc1;letter-spacing:2px}h1{font-size:32px;margin-bottom:10px}p{color:#b6bfd4;line-height:1.7}select,button{font:inherit;border-radius:14px;border:1px solid #424a60;background:#262d3e;color:inherit;padding:12px 20px}select{width:100%;margin:20px 0}#talk{display:block;width:180px;height:180px;border-radius:50%;margin:28px auto;background:#6757cf;border:2px solid #a497f2;font-size:22px;touch-action:none;user-select:none}#talk:active{background:#a3527c;transform:scale(.98)}button:disabled{opacity:.4}#status{min-height:52px}#metric{font-size:13px;color:#8794b3}footer{margin-top:24px;font-size:13px;color:#8794b3}</style>
<main><small>GROK BOT · VOZ NATIVA</small><h1 id="name">Grok Bot</h1><p>Mantén pulsado para hablar y suelta para oír la respuesta.</p><select id="bots" aria-label="Elegir Bot"></select><button id="connect">Conectar</button> <button id="hangup" disabled>Colgar</button><button id="talk" disabled>Mantén para hablar</button><p id="status" role="status">Leyendo el estado…</p><div id="metric"></div><footer>Al soltar termina la grabación · Mantén pulsado durante la respuesta para interrumpir<br>Al colgar, la conversación se guarda en el chat del Bot elegido<br><a href="/setup" style="color:#f6a0ca">Configurar Wi-Fi y conexión del dispositivo</a></footer></main>
<script>const controlToken = "${token}";
const $=id=>document.getElementById(id);let chain=Promise.resolve(),held=false,listed='';
function command(action,extra={}){chain=chain.catch(()=>{}).then(async()=>{const r=await fetch('/command',{method:'POST',headers:{'content-type':'application/json','x-bridge-control':controlToken},body:JSON.stringify({action,...extra}),keepalive:true});if(!r.ok)throw Error('La acción no se completó');});return chain;}
$('connect').onclick=()=>command('connect');$('hangup').onclick=()=>command('hangup');$('bots').onchange=()=>command('select',{id:$('bots').value});
function release(){if(!held)return;held=false;command('release');}
$('talk').onpointerdown=e=>{if(e.button!==0||held)return;e.preventDefault();held=true;$('talk').setPointerCapture(e.pointerId);command('press');};
$('talk').onpointerup=release;$('talk').onpointercancel=release;window.onblur=release;window.addEventListener('pagehide',()=>{release();${device?'':"command('hangup');"}});
async function refresh(){try{const s=await(await fetch('/state')).json();$('name').textContent=s.bot;if(s.device){document.querySelector('main>p').textContent='Mantén BOOT en el dispositivo para hablar y suelta para oír la respuesta.';$('talk').textContent='Se controla con el botón del dispositivo';}const names={disconnected:'Sin conectar',connecting:'Conectando la voz nativa…',ready:'Conectado: mantén pulsado para hablar',recording:'Escuchando; suelta para que responda',finishing:'Enviando la voz…',waiting:'Esperando la respuesta…',speaking:'Reproduciendo la respuesta',failed:'La conexión no se completó'};$('status').textContent=(names[s.state]||s.state)+(s.fault?' · '+s.fault:'')+(s.taskReturnAvailable===false?' · Recuperando el canal de tareas':'');$('connect').disabled=!['disconnected','failed'].includes(s.state);$('hangup').disabled=s.state==='disconnected';$('talk').disabled=s.device||!['ready','recording','waiting','speaking'].includes(s.state);$('bots').disabled=!['disconnected','failed'].includes(s.state);const key=JSON.stringify(s.bots);if(key!==listed){listed=key;$('bots').replaceChildren(...s.bots.map(b=>{const o=document.createElement('option');o.value=b.id;o.textContent=b.name;return o;}));}$('bots').value=s.botId;$('metric').textContent=s.firstAudioMs==null?'':'Primer audio en '+(s.firstAudioMs/1000).toFixed(2)+' s';}catch{$('status').textContent='El puente se ha detenido';$('talk').disabled=true;}setTimeout(refresh,500);}refresh();</script></html>`;
}
