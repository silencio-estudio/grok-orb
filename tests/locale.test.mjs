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
