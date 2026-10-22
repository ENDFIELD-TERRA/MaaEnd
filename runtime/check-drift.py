#!/usr/bin/env python3
"""Compare the actual checkout against runtime.json's recorded baseline. Read-only; no network."""
from __future__ import annotations

import argparse
import json
import subprocess
import sys
from pathlib import Path

AUDITED = ["AGENTS.md", ".agents/skills", ".github/workflows/issue-ai-analysis.yml",
           ".github/workflows/issue-auto-label.yml", ".github/workflows/issue-upload-check.yml",
           "docs/zh_cn/developers", "pyproject.toml", "tests/MaaEndTestset"]


def git(repo: Path, *args: str) -> subprocess.CompletedProcess:
    return subprocess.run(["git", "--no-optional-locks", "-c", "core.fsmonitor=false", "-C", str(repo), *args],
                          capture_output=True, encoding="utf-8", errors="replace", timeout=30)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repo", required=True, type=Path)
    parser.add_argument("--json", action="store_true", help="Machine-readable output")
    args = parser.parse_args()
    repo = args.repo.resolve(strict=True)
    runtime = json.loads((repo / ".qoder/maaend-private/runtime.json").read_text(encoding="utf-8-sig"))
    head = git(repo, "rev-parse", "HEAD")
    if head.returncode:
        print("STOP: " + head.stderr.strip(), file=sys.stderr)
        return 2
    head = head.stdout.strip()
    recorded = runtime.get("source_head")
    snapshot = runtime.get("snapshot_ref")
    skills_changed = git(repo, "diff", "--stat", snapshot, head, "--", ".agents/skills")
    rules_changed = git(repo, "diff", "--stat", snapshot, head, "--", "AGENTS.md")
    wf_changed = git(repo, "diff", "--stat", snapshot, head, "--", ".github/workflows")
    docs_changed = git(repo, "diff", "--stat", snapshot, head, "--", "docs/zh_cn/developers", "pyproject.toml")
    sub_changed = git(repo, "diff", snapshot, head, "--", "tests/MaaEndTestset")
    result = {
        "head": head,
        "recorded_head": recorded,
        "snapshot_ref": snapshot,
        "head_drift": head != recorded,
        "skills_changed_vs_snapshot": bool(skills_changed.stdout.strip()),
        "agents_md_changed_vs_snapshot": bool(rules_changed.stdout.strip()),
        "workflows_changed_vs_snapshot": bool(wf_changed.stdout.strip()),
        "docs_changed_vs_snapshot": bool(docs_changed.stdout.strip()),
        "testset_gitlink_changed": bool(sub_changed.stdout.strip()),
    }
    result["re_sync_required"] = any(result[k] for k in [
        "skills_changed_vs_snapshot", "agents_md_changed_vs_snapshot",
        "workflows_changed_vs_snapshot", "docs_changed_vs_snapshot", "testset_gitlink_changed"])
    if result["head_drift"]:
        result["re_audit_required"] = True
    if args.json:
        print(json.dumps(result, ensure_ascii=False, indent=2))
    else:
        for key, value in result.items():
            print(f"{key}: {value}")
    return 0 if not result["re_sync_required"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
