import { name } from './vendor/gdp/index.ts';
import { buildReviewRecord, verifyRun, type CandidateBuild, type RunIdentity, type VerificationSuite } from './proofs/proof-boundary.ts';
import { pendingRuntimeReader } from './runtime-bridge.ts';

/** Type example only. Calling it still rejects until the real bridge is integrated. */
export function prepareReview(run: RunIdentity, build: CandidateBuild, suite: VerificationSuite) {
  return name(Object.freeze({ ...run }), Object.freeze({ ...build }), Object.freeze({ ...suite }), async (r, b, s) => {
    const proof = await verifyRun(r, b, s, pendingRuntimeReader);
    if (proof === null) throw new Error('The current runtime evidence did not qualify this exact run/build/suite.');
    return buildReviewRecord(r, b, s, proof);
  });
}
