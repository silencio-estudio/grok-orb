import { test } from 'node:test';
import assert from 'node:assert/strict';
import { bridgeLocale, utcOffsetMinutes } from '../src/locale.mjs';
import { petMessage, encodePetMessage } from '../src/pet-protocol.mjs';
import { Backend } from '../src/backend.mjs';

test('bridge defaults to Spanish voice and Madrid time, configurable by environment', () => {
  assert.deepEqual(bridgeLocale({}), { language: 'es', spoken: 'Spanish', hint: 'es', timeZone: 'Europe/Madrid' });
  assert.deepEqual(bridgeLocale({ GROK_LANGUAGE: 'zh', GROK_TIMEZONE: 'Asia/Shanghai' }),
    { language: 'zh', spoken: 'Chinese', hint: 'zh', timeZone: 'Asia/Shanghai' });
  assert.throws(() => bridgeLocale({ GROK_LANGUAGE: 'klingon' }), /GROK_LANGUAGE/);
  assert.throws(() => bridgeLocale({ GROK_TIMEZONE: 'Mars/Olympus' }), /GROK_TIMEZONE/);
});

test('UTC offset follows daylight saving for the configured zone', () => {
  assert.equal(utcOffsetMinutes('Europe/Madrid', Date.UTC(2026, 0, 15, 12)), 60);
  assert.equal(utcOffsetMinutes('Europe/Madrid', Date.UTC(2026, 6, 15, 12)), 120);
  // 29 de marzo de 2026: a la 01:00 UTC Madrid pasa de UTC+1 a UTC+2.
  assert.equal(utcOffsetMinutes('Europe/Madrid', Date.UTC(2026, 2, 29, 0, 59)), 60);
  assert.equal(utcOffsetMinutes('Europe/Madrid', Date.UTC(2026, 2, 29, 1, 0)), 120);
  assert.equal(utcOffsetMinutes('Atlantic/Canary', Date.UTC(2026, 0, 15, 12)), 0);
  assert.equal(utcOffsetMinutes('Asia/Kolkata', Date.UTC(2026, 0, 15, 12)), 330);
  assert.equal(utcOffsetMinutes('America/New_York', Date.UTC(2026, 0, 15, 12)), -300);
});

test('clock message carries an optional, bounded UTC offset', () => {
  const clock = { op: 'clock', generation: 3, epochMs: 1800000000000 };
  assert.deepEqual(petMessage(encodePetMessage(clock)), clock);
  assert.deepEqual(petMessage(encodePetMessage({ ...clock, offsetMin: 120 })), { ...clock, offsetMin: 120 });
  for (const offsetMin of [841, -841, 1.5, '60', null])
    assert.throws(() => petMessage(Buffer.from(JSON.stringify({ ...clock, offsetMin }))), /PET_PROTOCOL/);
});

test('native RPC sends the configured timezone header', async () => {
  let headers;
  const backend = new Backend({ accessToken: 'token' }, { timeZone: 'Europe/Madrid',
    fetch: async (_url, options) => { headers = options.headers; return Response.json({}); } });
  await backend.rpc('EnsureSandBox', {});
  assert.equal(headers['x-cursor-timezone'], 'Europe/Madrid');
});

test('default bot is Grok Orb (decision D2)', async () => {
  const { DEFAULT_BOT } = await import('../src/locale.mjs');
  assert.equal(DEFAULT_BOT, 'Grok Orb');
});

test('--language and --time-zone override the environment and are validated', async () => {
  const { localeEnv, bridgeLocale } = await import('../src/locale.mjs');
  const env = localeEnv(['serve', '--language', 'zh', '--time-zone', 'Asia/Kolkata'], { GROK_LANGUAGE: 'es', GROK_TIMEZONE: 'Europe/Madrid' });
  assert.deepEqual(bridgeLocale(env), { language: 'zh', spoken: 'Chinese', hint: 'zh', timeZone: 'Asia/Kolkata' });
  assert.deepEqual(bridgeLocale(localeEnv([], {})), bridgeLocale({}));
  assert.throws(() => localeEnv(['--language', 'fr'], {}), /GROK_LANGUAGE/);
  assert.throws(() => localeEnv(['--time-zone', 'Mars/Olympus'], {}), /GROK_TIMEZONE/);
});

test('voice session asks the service for the configured spoken language', async () => {
  const { EventEmitter } = await import('node:events');
  const { NativeBotSession } = await import('../src/native-bot.mjs');
  const { bridgeLocale } = await import('../src/locale.mjs');
  class Voice extends EventEmitter { async open(args) { this.args = args; } close() { this.emit('closed'); } }
  const asked = [];
  for (const GROK_LANGUAGE of ['es', 'zh']) {
    const calls = [], voice = new Voice();
    const backend = {
      rpc: async (method, body) => { calls.push({ method, body }); return method === 'VoiceCallHarnessSession' ? { instructions: '', tools: [] } : { model: 'fixture' }; },
      gatewayCall: async () => ({ entries: [] })
    };
    const session = new NativeBotSession({ backend, bot: { id: 'orb', name: 'Grok Orb' }, voice, pollMs: 60000, locale: bridgeLocale({ GROK_LANGUAGE }) });
    await session.open(); await session.close();
    asked.push([calls.find(call => call.method === 'VoiceCallHarnessSession').body.spokenLanguage, voice.args.language]);
  }
  assert.deepEqual(asked, [['Spanish', 'es'], ['Chinese', 'zh']]);
});

test('device bridge sends the local offset with every clock', async () => {
  const { EventEmitter, once } = await import('node:events');
  const { setTimeout: delay } = await import('node:timers/promises');
  const tls = (await import('node:tls')).default;
  const { startDeviceServer } = await import('../src/device-server.mjs');
  const { encodeFrame, FrameDecoder } = await import('../src/device-wire.mjs');
  class Session extends EventEmitter { voice = { turn: 0 }; async open() {} async close() { return { saved: true }; } }
  const key = Buffer.alloc(32, 7);
  // Asia/Kolkata no tiene horario de verano: el desfase es siempre +5:30.
  const server = await startDeviceServer({ bot: { name: 'fixture' }, key, host: '127.0.0.1', port: 0, bridgeId: 'b'.repeat(32),
    timeZone: 'Asia/Kolkata', sessionFactory: () => new Session() });
  const socket = tls.connect({ host: '127.0.0.1', port: server.port, minVersion: 'TLSv1.2', maxVersion: 'TLSv1.2',
    ciphers: 'PSK-AES128-GCM-SHA256', rejectUnauthorized: false, pskCallback: () => ({ identity: 'grok-esp32', psk: key }) });
  const frames = [], decoder = new FrameDecoder(frame => frames.push(frame));
  socket.on('data', chunk => decoder.push(chunk));
  try {
    await once(socket, 'secureConnect');
    socket.write(encodeFrame(2, 0, Buffer.from('{"op":"hello","v":2,"generation":3}'))); await delay(30);
    const clocks = frames.filter(f => f.type === 2 && f.payload[0] === 123).map(f => JSON.parse(f.payload)).filter(m => m.op === 'clock');
    assert.equal(clocks.length, 1);
    assert.equal(clocks[0].offsetMin, 330);
  } finally { socket.destroy(); await server.close(); }
});
