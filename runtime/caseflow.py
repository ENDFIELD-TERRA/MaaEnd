#!/usr/bin/env python3
"""Private, single-writer case records for Qoder Agent Teams. No network or model calls."""
from __future__ import annotations

import argparse
from contextlib import contextmanager
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import sys
import tempfile


def canonical(value):
    return json.dumps(value, sort_keys=True, ensure_ascii=False, separators=(",", ":")).encode()


def sha(data):
    return hashlib.sha256(data).hexdigest()


def read(path):
    return json.loads(Path(path).read_text(encoding="utf-8-sig"))


def write(path, value):
    for part in [path, *path.parents]:
        if part.is_symlink() or (part.exists() and getattr(part.lstat(), "st_file_attributes", 0) & 0x400):
            raise ValueError("Output link/reparse point requires manual review")
    path.parent.mkdir(parents=True, exist_ok=True)
    fd, temporary = tempfile.mkstemp(dir=path.parent, prefix=".case-")
    try:
        with os.fdopen(fd, "wb") as out:
            out.write(json.dumps(value, ensure_ascii=False, indent=2).encode() + b"\n")
        os.replace(temporary, path)
    finally:
        if os.path.exists(temporary):
            os.unlink(temporary)


def scoped(root, relative):
    if not isinstance(relative, str) or not relative or Path(relative).is_absolute() or "\\" in relative:
        raise ValueError("Use a nonempty relative POSIX path")
    path = root / relative
    if not path.resolve().is_relative_to(root.resolve()) or ".." in Path(relative).parts:
        raise ValueError("Path escapes its declared root")
    for part in [path, *path.parents]:
        if part == root.parent:
            break
        if part.is_symlink() or (part.exists() and getattr(part.lstat(), "st_file_attributes", 0) & 0x400):
            raise ValueError("Links/reparse points require manual review")
    return path


def git(repo, *args):
    result = subprocess.run(["git", "--no-optional-locks", "-c", "core.fsmonitor=false", "-C", str(repo), *args],
                            capture_output=True, timeout=20)
    if result.returncode:
        raise ValueError(result.stderr.decode(errors="replace"))
    return result.stdout


@contextmanager
def locked(folder):
    folder.mkdir(parents=True, exist_ok=True)
    lock = folder / ".write-lock"
    fd = os.open(lock, os.O_CREAT | os.O_EXCL | os.O_WRONLY, 0o600)
    os.close(fd)
    try:
        yield
    finally:
        lock.unlink(missing_ok=True)


def validate_report(report):
    """Enforce report.schema.json without executing or installing a schema library."""
    fields = {"phase", "generation", "candidate_id", "author", "status", "summary", "artifacts"}
    if not isinstance(report, dict) or set(report) != fields:
        raise ValueError("Report must contain exactly the declared schema fields")
    if report["phase"] not in ["analysis", "integration", "review", "validation"] or report["status"] not in ["passed", "failed", "blocked", "not_run"]:
        raise ValueError("Unknown report phase or status")
    if type(report["generation"]) is not int or report["generation"] < 1:
        raise ValueError("Report generation must be a positive integer, not a boolean")
    candidate_id = report["candidate_id"]
    if candidate_id is not None and (not isinstance(candidate_id, str) or not re.fullmatch(r"[0-9a-f]{64}", candidate_id)):
        raise ValueError("Report candidate_id must be null or a lowercase SHA256")
    for field in ["author", "summary"]:
        if not isinstance(report[field], str) or not report[field].strip():
            raise ValueError("Report " + field + " must be a nonempty string")
    entries = report["artifacts"]
    if not isinstance(entries, list) or not entries:
        raise ValueError("At least one evidence artifact is required")
    for entry in entries:
        if not isinstance(entry, dict) or set(entry) != {"path", "sha256"}:
            raise ValueError("Artifacts must contain exactly path and sha256")
        if not isinstance(entry["path"], str) or not entry["path"].strip():
            raise ValueError("Artifact path must be a nonempty string")
        if not isinstance(entry["sha256"], str) or not re.fullmatch(r"[0-9a-f]{64}", entry["sha256"]):
            raise ValueError("Artifact sha256 must be a lowercase SHA256")


def checked_artifacts(root, entries):
    if not isinstance(entries, list) or not entries:
        raise ValueError("At least one nonempty evidence artifact is required")
    for entry in entries:
        path = scoped(root, entry["path"])
        data = path.read_bytes()
        if not data or sha(data) != entry["sha256"]:
            raise ValueError("Missing/empty/changed evidence artifact: " + entry["path"])


def current_candidate(repo, candidate):
    if git(repo, "rev-parse", "HEAD").decode().strip() != candidate["head"]:
        raise ValueError("Candidate HEAD changed; freeze and review a new candidate")
    if sha(git(repo, "diff", "--no-ext-diff", "--binary", "HEAD")) != candidate["diff_sha256"]:
        raise ValueError("Candidate diff changed; previous reports are stale")
    for relative, expected in candidate["files"].items():
        if sha(scoped(repo, relative).read_bytes()) != expected:
            raise ValueError("Candidate file changed: " + relative)


def checked_reports(folder, state, phases=None):
    for phase, index in state["reports"].items():
        if phases is not None and phase not in phases:
            continue
        report = read(scoped(folder, index["record"]))
        validate_report(report)
        if sha(canonical(report)) != index["sha256"]:
            raise ValueError("Recorded report changed: " + phase)
        checked_artifacts(folder, report["artifacts"])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repo", required=True, type=Path)
    parser.add_argument("--case", help="Stable private case id")
    sub = parser.add_subparsers(dest="command", required=True)
    intake = sub.add_parser("intake")
    intake.add_argument("--event", required=True, type=Path)
    freeze = sub.add_parser("freeze")
    freeze.add_argument("--manifest", required=True, type=Path)
    record = sub.add_parser("record")
    record.add_argument("--report", required=True, type=Path)
    draft = sub.add_parser("draft")
    draft.add_argument("--body", required=True, type=Path)
    draft.add_argument("--target", required=True)
    draft.add_argument("--action", required=True, choices=["analysis_comment", "pr", "push", "issue", "release"])
    sub.add_parser("status")
    args = parser.parse_args()
    repo = args.repo.resolve(strict=True)
    if Path(git(repo, "rev-parse", "--show-toplevel").decode().strip()).resolve() != repo:
        raise ValueError("--repo must name the current worktree root")
    private = scoped(repo, ".qoder/maaend-private")
    if not (private / "runtime.json").is_file():
        raise ValueError("Install the private configuration before recording a case")
    event = read(args.event) if args.command == "intake" else None
    if event:
        payload = event["payload"]
        issue = payload["issue"]
        comment = payload.get("comment", {})
        trigger = event["event_name"]
        eligible = (trigger == "issues" and payload.get("action") == "opened") or (
            trigger == "issue_comment" and payload.get("action") == "created"
            and "@MaaEndBot" in comment.get("body", "") and comment.get("user", {}).get("type") != "Bot") or trigger == "manual"
        if not eligible or "pull_request" in issue:
            print(json.dumps({"accepted": False, "reason": "Event is not an eligible issue-analysis request"}))
            return 0
        repository = payload["repository"]["full_name"]
        if not re.fullmatch(r"[A-Za-z0-9_.-]+/[A-Za-z0-9_.-]+", repository):
            raise ValueError("Invalid repository name")
        number = issue["number"]
        if not isinstance(number, int) or isinstance(number, bool) or number < 1:
            raise ValueError("Invalid issue number")
        case_id = args.case or repository.replace("/", "-").lower() + "-" + str(number)
    else:
        case_id = args.case
    if not case_id or not re.fullmatch(r"[a-z0-9][a-z0-9-]{0,79}", case_id):
        raise ValueError("A safe --case id is required")
    folder = scoped(private, "cases/" + case_id)
    with locked(folder):
        state_path = folder / "case.json"
        state = read(state_path) if state_path.exists() else None
        if args.command == "intake":
            if state and (state["repository"], state["issue_number"]) != (repository, number):
                raise ValueError("Case id belongs to a different issue")
            event_id = sha(canonical({"event": trigger, "action": payload.get("action"), "repository": repository,
                                      "issue": number, "comment": comment.get("id"),
                                      "updated": comment.get("updated_at", issue.get("updated_at")),
                                      "body": comment.get("body", issue.get("body", ""))}))
            if state and event_id in state["events"]:
                print(json.dumps({"accepted": True, "deduplicated": True, "case": case_id, "generation": state["generation"]}))
                return 0
            if not state:
                state = {"schema_version": 1, "case_id": case_id, "repository": repository, "issue_number": number,
                         "baseline_head": git(repo, "rev-parse", "HEAD").decode().strip(),
                         "generation": 0, "events": [], "reports": {}, "candidate": None,
                         "upstream": {"issue_state": issue.get("state", "unknown"), "merge": "unobserved", "release": "unobserved"}}
            state["generation"] += 1
            state["events"].append(event_id)
            state["reports"] = {}
            state["upstream"]["issue_state"] = issue.get("state", "unknown")
            write(folder / "inputs" / (event_id + ".json"), event)
            # Input is untrusted data, never a command, approval or authority.
            write(state_path, state)
            print(json.dumps({"accepted": True, "case": case_id, "generation": state["generation"], "public_write": False}))
            return 0
        if state is None:
            raise ValueError("Import an issue event first")
        if args.command == "freeze":
            manifest = read(args.manifest)
            if not manifest.get("build_id") or not isinstance(manifest.get("files"), dict) or not manifest["files"]:
                raise ValueError("Manifest needs a build_id and nonempty file:sha256 mapping")
            for relative, expected in manifest["files"].items():
                if relative.startswith(".qoder/") or not isinstance(expected, str) or sha(scoped(repo, relative).read_bytes()) != expected:
                    raise ValueError("Candidate source/build file hash is invalid: " + relative)
            candidate = {"build_id": manifest["build_id"], "files": manifest["files"],
                         "head": git(repo, "rev-parse", "HEAD").decode().strip(),
                         "diff_sha256": sha(git(repo, "diff", "--no-ext-diff", "--binary", "HEAD"))}
            candidate["id"] = sha(canonical(candidate))
            state["candidate"] = candidate
            state["reports"] = {}
            write(state_path, state)
            print(json.dumps({"candidate_id": candidate["id"], "generation": state["generation"]}))
            return 0
        candidate = state["candidate"]
        if candidate:
            current_candidate(repo, candidate)
        if args.command == "record":
            report = read(args.report)
            validate_report(report)
            phase = report["phase"]
            if report.get("generation") != state["generation"] or report.get("candidate_id") != (candidate["id"] if candidate else None):
                raise ValueError("Stale report generation/candidate")
            checked_artifacts(folder, report.get("artifacts"))
            if phase != "analysis" and candidate is None:
                raise ValueError("Freeze a real candidate before recording product acceptance")
            if phase == "validation" and report["status"] == "passed":
                checked_reports(folder, state, ["integration", "review"])
                if any(state["reports"].get(key, {}).get("status") != "passed" for key in ["integration", "review"]):
                    raise ValueError("Formal validation follows integration and independent review")
            if phase in ["integration", "review"] and report["status"] == "passed":
                other = "review" if phase == "integration" else "integration"
                if state["reports"].get(other, {}).get("author") == report["author"]:
                    raise ValueError("Integration and independent review must have different actual authors")
            downstream = {"analysis": ["integration", "review", "validation"], "integration": ["review", "validation"], "review": ["validation"], "validation": []}
            if state["reports"].get(phase, {}).get("sha256") != sha(canonical(report)):
                for stale in downstream[phase]:
                    state["reports"].pop(stale, None)
            relative = "reports/" + phase + "-" + sha(canonical(report)) + ".json"
            write(folder / relative, report)
            state["reports"][phase] = {"status": report["status"], "record": relative, "sha256": sha(canonical(report)), "author": report["author"]}
            write(state_path, state)
        elif args.command == "draft":
            checked_reports(folder, state)
            body = args.body.read_bytes()
            if not body.strip():
                raise ValueError("Public draft must be nonempty")
            draft = {"action": args.action, "target": args.target, "body_sha256": sha(body),
                     "candidate_id": candidate["id"] if candidate else None, "generation": state["generation"],
                     "approval": "Required through the actual user interaction tool; this record is not approval"}
            key = sha(canonical(draft))
            (folder / "drafts").mkdir(exist_ok=True)
            body_path = folder / "drafts" / (key + ".md")
            scoped(folder, "drafts/" + body_path.name).write_bytes(body)
            write(folder / "drafts" / (key + ".json"), draft)
            print(json.dumps({"draft": key, "published": False, "approval_required": True}))
            return 0
        checked_reports(folder, state)
        statuses = {k: v["status"] for k, v in state["reports"].items()}
        print(json.dumps({"case": case_id, "generation": state["generation"], "candidate": candidate["id"] if candidate else None,
                          "reports": statuses, "ready_for_human_review": all(statuses.get(p) == "passed" for p in ["integration", "review", "validation"]),
                          "upstream": state["upstream"], "public_write": False}, ensure_ascii=False))
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (KeyError, ValueError, OSError, subprocess.TimeoutExpired, TypeError) as error:
        print("BLOCKED: " + str(error), file=sys.stderr)
        raise SystemExit(2)
