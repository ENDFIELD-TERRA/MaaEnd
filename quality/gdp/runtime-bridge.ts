import type { TrustedRuntimeReader } from './proofs/proof-boundary.ts';

/** Wire to the authenticated real runner after full integration; never invent a Passed snapshot. */
export const pendingRuntimeReader: TrustedRuntimeReader = {
  async readSnapshot(): Promise<never> {
    throw new Error('Pending: no authenticated live authorization/result reader has been integrated.');
  },
};
