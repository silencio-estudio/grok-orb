import { test } from 'node:test';
import assert from 'node:assert/strict';
import { execFileSync } from 'node:child_process';
import { mkdtempSync, rmSync } from 'node:fs';
import { tmpdir } from 'node:os';
import { join } from 'node:path';
test('orb motion: ease-io2 curve, springs that chain without jumps, timed tweens', () => {
  const temp = mkdtempSync(join(tmpdir(), 'grok-pet-motion-'));
  try {
    execFileSync('/usr/bin/cc', ['-std=c11', '-Wall', '-Wextra', '-Werror', '-fsanitize=address,undefined', '-g',
      '-Ifirmware/pet/components/pet_ui', 'tests/pet_motion_harness.c', 'firmware/pet/components/pet_ui/pet_motion.c', '-lm', '-o', join(temp, 'motion')], { stdio: 'pipe' });
    assert.match(execFileSync(join(temp, 'motion'), { encoding: 'utf8' }), /PET_MOTION_OK/);
  } finally { rmSync(temp, { recursive: true, force: true }); }
});
