import { test } from 'node:test';
import assert from 'node:assert/strict';
import { setupPage } from '../src/setup-page.mjs';
import { startControlServer } from '../src/control.mjs';

const scripts = html => [...html.matchAll(/<script>([\s\S]*?)<\/script>/g)].map(match => match[1]);

test('setup and voice pages ship scripts the browser can parse', async () => {
  for (const source of scripts(setupPage('token'))) assert.doesNotThrow(() => new Function(source));
  const server = await startControlServer({ port: 0, snapshot: () => ({}), command: async () => {} });
  try {
    const html = await (await fetch(`http://127.0.0.1:${server.port}/`)).text();
    assert(scripts(html).length);
    for (const source of scripts(html)) assert.doesNotThrow(() => new Function(source));
  } finally { await server.close(); }
});
