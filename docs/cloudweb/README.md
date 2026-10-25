# CloudWeb planning docs

This directory is the public snapshot of the CloudWeb planning set:

- `roadmap.md` — single human-readable progress source (CW0–CW10, CW0–CW7 closure first).
- `roadmap.json` — machine-readable mirror of the same status.
- `plan.md` — contracts, acceptance criteria and first tasks for CW0–CW7 and FIX-RS.
- `cw8-cw10-plan.md` — CW8 full task qualification, CW9 fault/long-run, CW10 packaging/upstream plan.
- `acceptance-plan.md` — acceptance checklists for CW0–CW7/FIX-RS and the gate review flow.
- `inventory-current.json` — generated task/option/preset/pretask inventory at the fixed MaaEnd baseline (49/426/3/2/1, 46 Windows).
- `task-matrix.json` / `task-matrix.md` — 46-row Windows task matrix, all rows Pending, with evidence schema.
- `fault-matrix.json` / `fault-matrix.md` — CW9-1 fault matrix: 8 domains / 51 rows, all Pending, with closure layers and evidence schema.
- `cw9-batch-a-report.md` / `cw9-batch-a-results.json` — CW9-2 Batch A first pass: 15 protocol-fixture rows, 6 pass / 4 partial / 3 fail / 2 blocked.
- `cw9-cw2-reevaluation.md` — CW2 re-evaluation: minimal stop pass, full contract partial; remaining long-RPC preemption, owned launcher, Host-death.
- `cw9-runbook.md` — reproducible Batch A/B run commands, environment boundaries and evidence rules.
- `cw9-batch-b-plan.md` / `cw9-batch-b-readiness.json` — Batch B preparation and first execution: 26 local-browser rows, 14 partial / 11 not_run / 1 blocked, 0 pass.
- `cw9-batch-b-report.md` — first Batch B execution: 4 new CDP tests passed (media freeze/repeat/screenshot, page reload, target close, browser kill); test-only harness, product diff unchanged. The raw harness diff and test logs stay in the private workspace.

Scope note:

- Task-level handoff files for individual follow-up sessions are kept in the owner's private local workspace and are intentionally **not** part of this repository.
- Local absolute paths and private fork names were replaced with placeholders (`<workspace>`, `<sidecar-fork>`, `<MXU-fork>`) when this snapshot was produced.
- The public snapshot is not a claim that G1/G2/G3/G4 are closed. CW0–CW7 are `partial`/`not_started` as recorded in the roadmap; the only real task success so far was preconditioned and must not be generalised.
- CW8-1/2 and CW9-1 matrices are definitions and evidence schemas, not executed test results. `fault-matrix.json` has 51 Pending rows; fixture/local-browser rows cannot close real-cloud claims, and no row may be marked Pass without the full evidence schema.
- CW9-2 Batch A is a first protocol-layer pass only: 6 pass / 4 partial / 3 fail / 2 blocked. It does not close CW9-2 or G4. CW2 minimal stop is now pass on real-cloud AutoFight evidence, but the full CW2 contract remains `partial` (long-RPC preemption per command type, owned launcher, Host-death reclamation open), so Batch A will be re-run after those items close.
- Batch B first execution is still not a row-level pass: 26 local-browser rows are 14 partial / 11 not_run / 1 blocked, 0 pass. Four new CDP tests passed with a temporary profile/port (media freeze/repeat/screenshot, page reload, target close, browser kill), but session/ledger integration and the remaining rows are open.

Update procedure (idempotent):

1. Regenerate the derived files from the private single source.
2. Apply the same placeholder replacements; verify no local absolute path or private fork name remains.
3. Copy them here and run `git status --porcelain docs/cloudweb`.
4. If there is no diff, the update is already current; commit only when the diff is non-empty.

The material authority remains the three 2026-10-24 review/FRD/coordinator documents referenced in `roadmap.md`; this directory is a derived snapshot, not a second source of truth.
