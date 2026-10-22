import { name } from './vendor/gdp/index.ts';
import { buildReviewRecord, verifyRun, type CandidateBuild, type RunIdentity, type VerificationSuite } from './proofs/proof-boundary.ts';
import { pendingRuntimeReader } from './runtime-bridge.ts';

/** This file is expected to fail tsc with missing-proof and mismatched-name errors. */
export function rejectedMixups(run: RunIdentity, build: CandidateBuild, suite: VerificationSuite) {
  return name(run, build, suite, async (r, b, s) => {
    const proof = await verifyRun(r, b, s, pendingRuntimeReader);
    if (!proof) return;
    buildReviewRecord(r, b, s);
    name(build, (otherBuild) => { buildReviewRecord(r, otherBuild, s, proof); });
    name(suite, (otherSuite) => { buildReviewRecord(r, b, otherSuite, proof); });
    name(run, (otherRun) => { buildReviewRecord(otherRun, b, s, proof); });
  });
}
