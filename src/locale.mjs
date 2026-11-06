// Idioma de voz y zona horaria del puente. Por defecto, español y
// Europe/Madrid; se cambian con GROK_LANGUAGE (es, en, zh) y GROK_TIMEZONE
// (cualquier zona IANA, p. ej. Atlantic/Canary).
export const LANGUAGES = {
  es: { spoken: 'Spanish', hint: 'es' },
  en: { spoken: 'English', hint: 'en' },
  zh: { spoken: 'Chinese', hint: 'zh' },
};
// Bot de Grok Bot por defecto (decisión D2); se cambia con --bot o GROK_BOT_NAME.
export const DEFAULT_BOT = 'Grok Orb';
export const DEFAULT_LANGUAGE = 'es';
export const DEFAULT_TIMEZONE = 'Europe/Madrid';

export function validTimeZone(timeZone) {
  try { new Intl.DateTimeFormat('en-US', { timeZone }); return typeof timeZone === 'string' && timeZone.length > 0; }
  catch { return false; }
}

// --language y --time-zone ganan a GROK_LANGUAGE y GROK_TIMEZONE. Devuelve el entorno
// resultante ya validado, para que cli.mjs lo aplique antes de crear nada.
export function localeEnv(args = [], env = process.env) {
  const option = name => { const index = args.indexOf(name); return index < 0 ? undefined : args[index + 1]; };
  const next = { ...env };
  if (option('--language') !== undefined) next.GROK_LANGUAGE = option('--language');
  if (option('--time-zone') !== undefined) next.GROK_TIMEZONE = option('--time-zone');
  bridgeLocale(next);
  return next;
}

export function bridgeLocale(env = process.env) {
  const language = env.GROK_LANGUAGE ?? DEFAULT_LANGUAGE, timeZone = env.GROK_TIMEZONE ?? DEFAULT_TIMEZONE;
  if (!Object.hasOwn(LANGUAGES, language)) throw Object.assign(new Error('Unsupported GROK_LANGUAGE'), { code: 'LANGUAGE_UNSUPPORTED' });
  if (!validTimeZone(timeZone)) throw Object.assign(new Error('Unknown GROK_TIMEZONE'), { code: 'TIMEZONE_UNKNOWN' });
  return { language, ...LANGUAGES[language], timeZone };
}

// Minutos que la zona va por delante de UTC en ese instante (horario de verano incluido).
export function utcOffsetMinutes(timeZone, epochMs = Date.now()) {
  const parts = Object.fromEntries(new Intl.DateTimeFormat('en-US', { timeZone, hourCycle: 'h23',
    year: 'numeric', month: 'numeric', day: 'numeric', hour: 'numeric', minute: 'numeric' })
    .formatToParts(new Date(epochMs)).map(part => [part.type, Number(part.value)]));
  const local = Date.UTC(parts.year, parts.month - 1, parts.day, parts.hour, parts.minute);
  return Math.round((local - Math.floor(epochMs / 60000) * 60000) / 60000);
}
