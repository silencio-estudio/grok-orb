// Compila y lanza el simulador de escritorio del orb (macOS + SDL2).
//   node sim/sim.mjs            ventana interactiva
//   node sim/sim.mjs --shots D  capturas PNG de cada estado en D, sin ventana
//   node sim/sim.mjs --selftest prueba de teclas y estados, sin ventana
//   node sim/sim.mjs --build    solo compilar
import { execFileSync, spawn } from 'node:child_process';
import { createHash } from 'node:crypto';
import { existsSync, mkdirSync, readdirSync, readFileSync, statSync, writeFileSync } from 'node:fs';
import { availableParallelism } from 'node:os';
import { dirname, join, relative } from 'node:path';
import { fileURLToPath } from 'node:url';

const root = join(dirname(fileURLToPath(import.meta.url)), '..');
const sim = join(root, 'sim'), build = join(root, '.build/sim'), lvgl = join(build, 'lvgl');
const LVGL_TAG = 'v9.5.0', LVGL_COMMIT = '85aa60d18b3d5e5588d7b247abf90198f07c8a63';
const firmware = join(root, 'firmware/pet/components');
const args = process.argv.slice(2);

function run(command, list, options = {}) { return (execFileSync(command, list, { encoding: 'utf8', ...options }) ?? '').trim(); }
function fail(message) { console.error(`Simulador: ${message}`); process.exit(1); }

if (process.platform !== 'darwin') fail('de momento solo está preparado para macOS.');
let sdlConfig;
try { sdlConfig = { cflags: run('sdl2-config', ['--cflags']).split(/\s+/), prefix: run('sdl2-config', ['--prefix']), libs: run('sdl2-config', ['--libs']).split(/\s+/) }; }
catch { fail('falta SDL2. Instálalo con: brew install sdl2'); }

if (!existsSync(join(lvgl, 'lvgl.h'))) {
  mkdirSync(build, { recursive: true });
  console.log(`Descargando LVGL ${LVGL_TAG} (una sola vez)…`);
  run('git', ['clone', '--quiet', '--depth', '1', '--branch', LVGL_TAG, 'https://github.com/lvgl/lvgl.git', lvgl], { stdio: 'inherit' });
}
// La misma versión que el firmware (firmware/pet/main/idf_component.yml).
if (run('git', ['-C', lvgl, 'rev-parse', 'HEAD']) !== LVGL_COMMIT) fail(`LVGL en ${relative(root, lvgl)} no es ${LVGL_TAG}; bórralo y vuelve a lanzar.`);

const flags = ['-std=gnu11', '-O2', '-g', '-DLV_CONF_INCLUDE_SIMPLE', '-DLV_LVGL_H_INCLUDE_SIMPLE',
  `-I${sim}`, `-I${join(sim, 'fakes')}`, `-I${lvgl}`, `-I${join(firmware, 'pet_ui')}`, `-I${join(firmware, 'pet_core')}`, `-I${join(firmware, 'pet_strings')}`,
  ...sdlConfig.cflags, `-I${join(sdlConfig.prefix, 'include')}`, '-Wno-unused-command-line-argument'];
const confHash = createHash('sha256').update(readFileSync(join(sim, 'lv_conf.h'))).update(flags.join(' ')).digest('hex').slice(0, 16);
const objects = join(build, `obj-${confHash}`);
mkdirSync(objects, { recursive: true });

const walk = dir => readdirSync(dir, { withFileTypes: true }).flatMap(entry =>
  entry.isDirectory() ? walk(join(dir, entry.name)) : entry.name.endsWith('.c') ? [join(dir, entry.name)] : []);
const ours = [join(sim, 'sim_main.c'), join(firmware, 'pet_ui/pet_ui.c'), join(firmware, 'pet_core/pet_core.c'),
  // Sin sdkconfig, pet_strings.h elige español: como en la placa, sin la fuente CJK.
  join(firmware, 'pet_ui/fonts/pet_font_latin_16.c')];
const newestHeader = Math.max(...[join(sim, 'fakes'), join(firmware, 'pet_ui'), join(firmware, 'pet_core')]
  .flatMap(dir => readdirSync(dir, { recursive: true }).filter(name => name.endsWith('.h')).map(name => statSync(join(dir, name)).mtimeMs)));
const sources = [...walk(join(lvgl, 'src')), ...ours];

const objectFor = source => join(objects, relative(root, source).replaceAll('/', '__').replace(/\.c$/, '.o'));
const stale = sources.filter(source => {
  const object = objectFor(source);
  if (!existsSync(object)) return true;
  const built = statSync(object).mtimeMs;
  return statSync(source).mtimeMs > built || (ours.includes(source) && newestHeader > built);
});

async function compile(list) {
  let next = 0, done = 0;
  const worker = async () => {
    while (next < list.length) {
      const source = list[next++];
      await new Promise((resolve, reject) => {
        const child = spawn('cc', [...flags, '-c', source, '-o', objectFor(source)], { stdio: ['ignore', 'inherit', 'inherit'] });
        child.on('exit', code => code === 0 ? resolve() : reject(new Error(relative(root, source))));
      });
      if (++done % 50 === 0 || done === list.length) process.stdout.write(`\rCompilando ${done}/${list.length}`);
    }
  };
  await Promise.all(Array.from({ length: Math.min(availableParallelism(), list.length) }, worker));
  if (list.length) process.stdout.write('\n');
}
try { await compile(stale); } catch (error) { fail(`no compila ${error.message}`); }

const binary = join(build, 'grok-orb-sim');
const linkStamp = join(objects, '.link');
if (stale.length || !existsSync(binary) || !existsSync(linkStamp) || readFileSync(linkStamp, 'utf8').trim() !== binary) {
  run('cc', ['-o', binary, ...sources.map(objectFor), ...sdlConfig.libs, '-lz', '-lm'], { stdio: 'inherit' });
  writeFileSync(linkStamp, binary);
}
if (args.includes('--build')) { console.log(relative(root, binary)); process.exit(0); }

const shots = args.indexOf('--shots');
const fps = args.indexOf('--fps');
const mode = args.includes('--selftest') ? ['--selftest'] : args.includes('--bench') ? ['--bench']
  : fps >= 0 ? ['--fps', args[fps + 1] ?? '5'] : shots >= 0 ? ['--shots', args[shots + 1] ?? join(build, 'shots')] : [];
const child = spawn(binary, mode, { cwd: root, stdio: 'inherit' });
child.on('exit', code => process.exit(code ?? 1));
