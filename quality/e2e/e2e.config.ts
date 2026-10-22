import type { E2EConfig } from 'e2e';
import { web } from '@e2e-dev/web';

const target = process.env.MAAEND_WEB_TARGET_URL;
if (!target) {
  throw new Error('Pending: set the reviewed real MAAEND_WEB_TARGET_URL before opening a session.');
}
const parsed = new URL(target);
if (!['http:', 'https:'].includes(parsed.protocol) || parsed.username || parsed.password) {
  throw new Error('Use a reviewed HTTP(S) target without credentials in the URL.');
}
if (process.env.E2E_TELEMETRY_DISABLED !== '1') {
  throw new Error('Set E2E_TELEMETRY_DISABLED=1 in the process that launches e2e.');
}

export default {
  projectId: 'maaend-private-web-qualification',
  targets: [{ name: 'web', engine: web({ browser: 'chromium' }), app: { url: target } }],
  tests: 'tests/**/*.e2e.ts',
  workers: 1,
  retries: 0,
  timeout: 60_000,
  launchTimeout: 30_000,
  actionTimeout: 10_000,
  assertionTimeout: 8_000,
  cleanupTimeout: 15_000,
  maxDuration: 90_000,
  cache: 'off',
  trace: 'off',
  video: 'off',
  output: '.e2e-private',
} satisfies E2EConfig;
