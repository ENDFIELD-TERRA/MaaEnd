import { defineProof, type Named, type Proof } from '../vendor/gdp/index.ts';

export interface RunIdentity {
  readonly owner: string;
  readonly generation: number;
  readonly targetId: string;
}
export interface CandidateBuild { readonly digest: string }
export interface VerificationSuite { readonly digest: string }

/** Implement in a reviewed private bridge using an authenticated, current runtime source. */
export interface TrustedRuntimeReader {
  readSnapshot(): Promise<unknown>;
}

const VerifiedRun = defineProof('VerifiedRun');
export interface VerifiedRun<R, B, S> extends Proof<'VerifiedRun', [R, B, S]> {
  readonly verifiedAt: number;
  readonly expiresAt: number;
}

function record(value: unknown): value is Record<string, unknown> {
  return typeof value === 'object' && value !== null && !Array.isArray(value);
}

function sameIdentity(value: Record<string, unknown>, run: RunIdentity, build: CandidateBuild, suite: VerificationSuite): boolean {
  return value.owner === run.owner && value.generation === run.generation && value.targetId === run.targetId &&
    value.buildDigest === build.digest && value.suiteDigest === suite.digest;
}

/**
 * This validates the reader's data, not the reader's authenticity.
 * The bridge owns provenance, candidate-to-process mapping, scope and atomicity.
 */
export async function verifyRun<R, B, S>(
  run: Named<R, RunIdentity>,
  build: Named<B, CandidateBuild>,
  suite: Named<S, VerificationSuite>,
  reader: TrustedRuntimeReader,
): Promise<VerifiedRun<R, B, S> | null> {
  const raw = await reader.readSnapshot();
  if (!record(raw) || !record(raw.authorization) || !record(raw.result)) return null;
  const authorization = raw.authorization;
  const result = raw.result;
  const now = Date.now();
  if (!run.value.owner.trim() || !run.value.targetId.trim() || !Number.isSafeInteger(run.value.generation) ||
      run.value.generation < 1 || !build.value.digest.trim() || !suite.value.digest.trim()) return null;
  if (!sameIdentity(authorization, run.value, build.value, suite.value) ||
      !sameIdentity(result, run.value, build.value, suite.value)) return null;
  if (authorization.allowed !== true || authorization.scope !== 'review-verification' ||
      typeof authorization.expiresAt !== 'number' || !Number.isSafeInteger(authorization.expiresAt) ||
      authorization.expiresAt <= now) return null;
  if (result.verdict !== 'Passed' || typeof result.measuredAt !== 'number' ||
      !Number.isSafeInteger(result.measuredAt) || result.measuredAt > now ||
      typeof result.expiresAt !== 'number' || !Number.isSafeInteger(result.expiresAt) ||
      result.expiresAt <= now || result.expiresAt <= result.measuredAt) return null;
  if (typeof result.plannedChecks !== 'number' || !Number.isSafeInteger(result.plannedChecks) ||
      result.plannedChecks < 1 || result.passedChecks !== result.plannedChecks ||
      result.failedChecks !== 0 || result.blockedChecks !== 0 || result.skippedChecks !== 0) return null;
  return Object.freeze({
    ...VerifiedRun.prove(run, build, suite),
    verifiedAt: now,
    expiresAt: Math.min(authorization.expiresAt, result.expiresAt),
  });
}

/** Produces a private review record only; it grants no publication or runtime control authority. */
export function buildReviewRecord<R, B, S>(
  run: Named<R, RunIdentity>,
  build: Named<B, CandidateBuild>,
  suite: Named<S, VerificationSuite>,
  proof: VerifiedRun<NoInfer<R>, NoInfer<B>, NoInfer<S>>,
) {
  if (Date.now() >= proof.expiresAt) throw new Error('Verification evidence expired; obtain a current runtime snapshot.');
  return Object.freeze({
    owner: run.value.owner,
    generation: run.value.generation,
    targetId: run.value.targetId,
    buildDigest: build.value.digest,
    suiteDigest: suite.value.digest,
    verifiedAt: proof.verifiedAt,
    expiresAt: proof.expiresAt,
  });
}
