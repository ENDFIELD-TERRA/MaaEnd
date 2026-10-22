import { name } from './vendor/gdp/index.ts';
import { buildReviewRecord, verifyRun, type CandidateBuild, type RunIdentity, type VerificationSuite } from './proofs/proof-boundary.ts';
import { pendingRuntimeReader } from './runtime-bridge.ts';

/** Negative type assertions; never invoke this with a live runner. */
export function rejectedMixups(run: RunIdentity, build: CandidateBuild, suite: VerificationSuite) {
  return name(run, build, suite, async (r, b, s) => {
    const proof = await verifyRun(r, b, s, pendingRuntimeReader);
    if (!proof) return;
    // @ts-expect-error A review requires evidence.
    buildReviewRecord(r, b, s);
    name(build, (otherBuild) => {
      // @ts-expect-error A separately named candidate cannot consume this proof.
      buildReviewRecord(r, otherBuild, s, proof);
    });
    name(suite, (otherSuite) => {
      // @ts-expect-error A separately named suite cannot consume this proof.
      buildReviewRecord(r, b, otherSuite, proof);
    });
    name(run, (otherRun) => {
      // @ts-expect-error A separately named owner/generation/target cannot consume this proof.
      buildReviewRecord(otherRun, b, s, proof);
    });
  });
}
