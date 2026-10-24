# CloudWeb CW0–CW7 planning docs

This directory is the public snapshot of the CloudWeb planning set:

- `roadmap.md` — single human-readable progress source (CW0–CW10, CW0–CW7 closure first).
- `roadmap.json` — machine-readable mirror of the same status.
- `plan.md` — contracts, acceptance criteria and first tasks for CW0–CW7 and FIX-RS.
- `cw8-cw10-plan.md` — CW8 full task qualification, CW9 fault/long-run, CW10 packaging/upstream plan.
- `acceptance-plan.md` — acceptance checklists for CW0–CW7/FIX-RS and the gate review flow.

Scope note:

- Task-level handoff files for individual follow-up sessions are kept in the owner's private local workspace and are intentionally **not** part of this repository.
- Local absolute paths and private fork names were replaced with placeholders (`<workspace>`, `<sidecar-fork>`, `<MXU-fork>`) when this snapshot was produced.
- The public snapshot is not a claim that G1/G2/G3 are closed. CW0–CW7 are `partial`/`not_started` as recorded in the roadmap; the only real task success so far was preconditioned and must not be generalised.

Update procedure (idempotent):

1. Regenerate the three files from the private single source.
2. Apply the same placeholder replacements.
3. Copy them here and run `git status --porcelain docs/cloudweb`.
4. If there is no diff, the update is already current; commit only when the diff is non-empty.

The material authority remains the three 2026-10-24 review/FRD/coordinator documents referenced in `roadmap.md`; this directory is a derived snapshot, not a second source of truth.
