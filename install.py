#!/usr/bin/env python3
"""Prepare private Qoder CN configuration without executing repository scripts."""
from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import stat
import subprocess
import sys
import tempfile

BUNDLE = Path(__file__).resolve().parent
AGENT_NAME = "perlica-maaend"
PRIVATE = ".qoder/maaend-private"


def digest(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def encoded(value: object) -> bytes:
    return (json.dumps(value, ensure_ascii=False, indent=2) + "\n").encode()


def git(repo: Path, *args: str) -> str:
    result = subprocess.run(
        ["git", "--no-optional-locks", "-c", "core.fsmonitor=false", "-C", str(repo), *args],
        capture_output=True, encoding="utf-8", errors="replace", timeout=20,
    )
    if result.returncode:
        raise ValueError(f"git {args[0]} failed ({result.returncode}): {result.stderr.strip()}")
    return result.stdout.rstrip("\n")


def reject_link(path: Path) -> None:
    if path.is_symlink() or (path.exists() and getattr(path.lstat(), "st_file_attributes", 0) & getattr(stat, "FILE_ATTRIBUTE_REPARSE_POINT", 0x400)):
        raise ValueError(f"Link/reparse point needs manual review: {path}")


def safe_bytes(path: Path) -> bytes:
    reject_link(path)
    return path.read_bytes()


def skill_name(data: bytes) -> str:
    text = data.decode("utf-8-sig")
    match = re.match(r"\A---\s*\n(.*?)\n---(?:\s*\n|$)", text, re.S)
    if not match:
        raise ValueError("SKILL.md lacks YAML frontmatter")
    names = re.findall(r"^name:\s*['\"]?([a-z0-9-]+)['\"]?\s*$", match[1], re.M)
    if len(names) != 1:
        raise ValueError("SKILL.md must have one lowercase name")
    return names[0]


def tree_files(root: Path) -> list[tuple[str, bytes]]:
    items = []
    reject_link(root)
    for path in sorted(root.rglob("*")):
        reject_link(path)
        if path.is_file():
            items.append((path.relative_to(root).as_posix(), safe_bytes(path)))
    return items


def mcp_servers() -> dict:
    prefix = ["/d", "/s", "/c", "npx"] if os.name == "nt" else []
    command = "cmd" if os.name == "nt" else "npx"
    common = {"command": command, "disabled": True, "trust": False, "timeout": 30000}
    return {
        "playwright": {**common, "args": prefix + ["-y", "@playwright/mcp@0.0.83", "--browser=chrome", "--isolated", "--no-webmcp"]},
        "chrome-devtools": {**common, "args": prefix + ["-y", "chrome-devtools-mcp@1.11.0", "--isolated", "--no-usage-statistics", "--no-performance-crux"],
                            "env": {"CHROME_DEVTOOLS_MCP_NO_UPDATE_CHECKS": "1"}},
    }


def atomic_write(path: Path, data: bytes) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    fd, tmp = tempfile.mkstemp(prefix=".qoder-install-", dir=path.parent)
    try:
        with os.fdopen(fd, "wb") as out:
            out.write(data)
        os.replace(tmp, path)
    finally:
        if os.path.exists(tmp):
            os.unlink(tmp)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repo", required=True, type=Path)
    parser.add_argument("--config-root", type=Path, default=Path(os.environ.get("QODERCN_CONFIG_DIR", str(Path.home() / ".qoder-cn"))))
    parser.add_argument("--global-agents", type=Path)
    parser.add_argument("--seed-global", action="store_true", help="Create baseline only when the selected global file is absent")
    parser.add_argument("--apply", action="store_true", help="Apply the displayed private-file plan")
    args = parser.parse_args()
    repo = args.repo.resolve(strict=True)
    config = args.config_root.expanduser().resolve()
    global_path = (args.global_agents or config / "AGENTS.md").expanduser().absolute()
    if Path(git(repo, "rev-parse", "--show-toplevel")).resolve() != repo:
        raise ValueError("--repo must be the Git worktree root")
    for path in [repo / ".qoder", repo / ".agents", repo / ".agents/skills"]:
        reject_link(path)
    for required in ["AGENTS.md", ".agents/skills", "docs/zh_cn/developers/coding-standards.md"]:
        if not (repo / required).exists():
            raise ValueError(f"Missing MaaEnd source: {required}")
    # Read-only baseline; no checkout, fetch, install, build or repository script.
    audit = {
        "head": git(repo, "rev-parse", "HEAD"),
        "branch": git(repo, "rev-parse", "--abbrev-ref", "HEAD"),
        "dirty": git(repo, "status", "--porcelain=v1"),
        "submodules": git(repo, "submodule", "status"),
        "recent_subjects": git(repo, "log", "-8", "--format=%h %s"),
        "remotes": re.sub(r"(https?://)[^/@\s]+:[^/@\s]+@", r"\1<redacted>@", git(repo, "remote", "-v")),
        "lockfiles": git(repo, "ls-files", "*lock*", "*go.sum"),
        "ci_files": git(repo, "ls-files", ".github/workflows/*"),
        "issue_lifecycle": "Pending: query the relevant remote issue/PR before implementation",
    }
    proposed: dict[Path, bytes] = {}
    if global_path.exists():
        global_bytes = safe_bytes(global_path)
    elif args.seed_global:
        global_bytes = safe_bytes(BUNDLE / "policy/global-baseline.md")
        proposed[global_path] = global_bytes
    else:
        raise ValueError("Global AGENTS.md is missing; pass --global-agents to the real file, or explicitly --seed-global")
    maps = []
    names: set[str] = set()
    roots = [(root, "repository", root.relative_to(repo).as_posix())
             for root in sorted((repo / ".agents/skills").iterdir()) if (root / "SKILL.md").exists()]
    for group, origin in [("browser-skills", "browser"), ("method-skills", "method"), ("workflow-skills", "workflow")]:
        roots += [(root, origin, group + "/" + root.name)
                  for root in sorted((BUNDLE / group).glob("*")) if (root / "SKILL.md").exists()]
    for root, origin, source in roots:
        reject_link(root)
        name = skill_name(safe_bytes(root / "SKILL.md"))
        if name in names:
            raise ValueError(f"Duplicate skill name: {name}")
        names.add(name)
        files = tree_files(root)
        manifest = {rel: digest(data) for rel, data in files}
        # CN user-level skills can shadow project skills. Refuse a differing definition.
        for user_root in (config / "skills").glob("*"):
            entry = user_root / "SKILL.md"
            if entry.is_file() and skill_name(safe_bytes(entry)) == name:
                if dict((p, digest(b)) for p, b in tree_files(user_root)) != manifest:
                    raise ValueError(f"User-level skill shadows project skill with different bytes: {name}")
        target = repo / ".qoder/skills" / name
        for rel, data in files:
            proposed[target / rel] = data
        maps.append({"name": name, "origin": origin, "source": source,
                     "installed": f".qoder/skills/{name}/SKILL.md", "files": manifest})
    if "perlica-style-reply" not in names or not any(m["origin"] == "repository" for m in maps):
        raise ValueError("Required MaaEnd/Perlica skills missing")
    agent_names = []
    templates = [BUNDLE / "templates/perlica-maaend.md", *sorted((BUNDLE / "team-templates").glob("*.md"))]
    for template in templates:
        definition = safe_bytes(template)
        name = skill_name(definition)
        if name in agent_names:
            raise ValueError("Duplicate custom definition: " + name)
        agent_names.append(name)
        content = definition.decode().replace("{{SKILLS}}", "\n".join("  - " + item for item in sorted(names)))
        proposed[repo / f".qoder/agents/{name}.md"] = content.encode()
    for path in sorted((BUNDLE / "policy").iterdir()):
        if path.is_file():
            proposed[repo / PRIVATE / path.name] = safe_bytes(path)
    for group in ["runtime", "schemas", "quality", "research"]:
        for relative, content in tree_files(BUNDLE / group):
            installed = relative if group == "runtime" else group + "/" + relative
            proposed[repo / PRIVATE / installed] = content
    proposed[repo / PRIVATE / "Start-Team.ps1"] = safe_bytes(BUNDLE / "Start-Team.ps1")
    command_paths = []
    for relative, content in tree_files(BUNDLE / "commands"):
        destination = ".qoder/commands/" + relative
        proposed[repo / destination] = content
        command_paths.append("/" + destination)
    proposed[repo / PRIVATE / "skill-map.json"] = encoded(maps)
    proposed[repo / PRIVATE / "runtime.json"] = encoded({
        "global_agents": str(global_path), "global_sha256": digest(global_bytes),
        "config_root": str(config), "source_head": audit["head"],
        "snapshot_ref": "654aa96253303931a64361827f18701565edd65d",
        "source_agents_sha256": digest(safe_bytes(repo / "AGENTS.md")),
        "mcp_status": "Definitions available; inspect effective enablement and permissions, local handshake pending",
        "team_status": "Seven role definitions prepared; native Teams TUI lifecycle pending local qualification",
    })
    receipt_path = repo / PRIVATE / "install-receipt.json"
    for path in [receipt_path, *receipt_path.parents]:
        reject_link(path)
    previous = json.loads(safe_bytes(receipt_path)) if receipt_path.exists() else {}
    if receipt_path.exists():
        if not isinstance(previous, dict) or previous.get("format") != "qoder-maaend-import-v1":
            raise ValueError("Existing receipt is not owned by this installer; preserve and review it")
        signed = {key: value for key, value in previous.items() if key != "integrity"}
        if previous.get("integrity") != digest(encoded(signed)):
            raise ValueError("Existing receipt was edited or corrupted; preserve and review it")
    old_managed = previous.get("managed", {})
    # Merge the two server entries without touching unrelated settings. JSONC needs manual review.
    settings_path = repo / ".qoder/settings.local.json"
    settings = json.loads(safe_bytes(settings_path).decode("utf-8-sig")) if settings_path.exists() else {}
    if not isinstance(settings, dict) or not isinstance(settings.get("mcpServers", {}), dict):
        raise ValueError("Existing settings.local.json must be a JSON object with mcpServers object")
    servers = settings.setdefault("mcpServers", {})
    for name, server in mcp_servers().items():
        if name in servers and servers[name] != server:
            # Preserve explicit local enablement/permission choices for the same pinned server.
            stable = lambda value: {k: v for k, v in value.items() if k not in ["disabled", "trust", "timeout"]}
            if not isinstance(servers[name], dict) or stable(servers[name]) != stable(server):
                raise ValueError(f"Existing MCP entry requires manual reconciliation: {name}")
        else:
            servers[name] = server
    proposed[settings_path] = encoded(settings)
    # Pin no automatic launch. Keep local additions and AGENTS unchanged.
    excludes = [f"/.qoder/agents/{name}.md" for name in agent_names]
    excludes += [f"/{PRIVATE}/", "/.qoder/settings.local.json", *command_paths]
    excludes += [f"/.qoder/skills/{name}/" for name in sorted(names)]
    exclude_path = Path(git(repo, "rev-parse", "--git-path", "info/exclude"))
    if not exclude_path.is_absolute():
        exclude_path = repo / exclude_path
    existing_exclude = safe_bytes(exclude_path).decode("utf-8") if exclude_path.exists() else ""
    begin, end = "# BEGIN private MaaEnd Qoder setup", "# END private MaaEnd Qoder setup"
    block = begin + "\n" + "\n".join(excludes) + "\n" + end
    if begin in existing_exclude:
        if existing_exclude.count(begin) != 1 or existing_exclude.count(end) != 1:
            raise ValueError("Malformed private ignore block")
        revised = re.sub(re.escape(begin) + r".*?" + re.escape(end), block, existing_exclude, flags=re.S)
    else:
        revised = existing_exclude + ("\n" if existing_exclude and not existing_exclude.endswith("\n") else "") + block + "\n"
    proposed[exclude_path] = revised.encode()
    receipt = {"format": "qoder-maaend-import-v1",
               "managed": {str(p): digest(data) for p, data in proposed.items() if p != exclude_path},
               "audit": audit, "skills": maps,
               "runtime_qualification": "Pending; no Qoder or MCP was started"}
    receipt["integrity"] = digest(encoded(receipt))
    proposed[receipt_path] = encoded(receipt)
    # Prevent tracked destination writes and modified/foreign configuration clobber.
    tracked = set(git(repo, "ls-files").splitlines())
    for target in proposed:
        if target.is_relative_to(repo) and target.relative_to(repo).as_posix() in tracked:
            raise ValueError(f"Refuse to overwrite a tracked file: {target.relative_to(repo)}")
        for parent in [target, *target.parents]:
            reject_link(parent)
        if target.is_relative_to(repo) and not target.resolve().is_relative_to(repo):
            raise ValueError(f"Destination resolves outside the checkout: {target}")
        if target.exists() and safe_bytes(target) != proposed[target]:
            special = target in [settings_path, exclude_path, receipt_path]
            if not special and old_managed.get(str(target)) != digest(safe_bytes(target)):
                raise ValueError(f"Protect an existing or edited file: {target}")
    # Detect stale skill assets rather than silently leaving an obsolete active copy.
    for mapping in maps:
        target = repo / ".qoder/skills" / mapping["name"]
        if target.exists():
            extras = {p for p, _ in tree_files(target)} - set(mapping["files"])
            if extras:
                raise ValueError(f"Stale/extra skill files need manual review: {mapping['name']}: {sorted(extras)}")
    for old_path in old_managed:
        path = Path(old_path)
        if path.is_relative_to(repo / ".qoder/skills") and path not in proposed and path.exists():
            raise ValueError(f"A removed source skill still has an installed file; review it manually: {path}")
    if any((config / "agents" / f"{name}.md").exists() for name in agent_names):
        raise ValueError("A user-level agent has the same name; inspect precedence before installing")
    changes = {p: data for p, data in proposed.items() if not p.exists() or safe_bytes(p) != data}
    summary = {"mode": "apply" if args.apply else "plan", "head": audit["head"],
               "repository_skills": sum(m["origin"] == "repository" for m in maps),
               "browser_skills": sum(m["origin"] == "browser" for m in maps),
               "method_and_workflow_skills": sum(m["origin"] in ["method", "workflow"] for m in maps),
               "custom_definitions": agent_names,
               "commands": command_paths,
               "skills": sorted(names), "changed_files": len(changes),
               "mcp": "Initial imports are disabled; existing local enablement preserved; runtime qualification pending",
               "issue_lifecycle": "Pending remote read during bootstrap"}
    print(json.dumps(summary, ensure_ascii=False, indent=2))
    for target in changes:
        print("WRITE " + str(target))
    if not args.apply:
        return 0
    # Preflight completed. Best-effort rollback of our byte writes on failure.
    backups = {p: safe_bytes(p) if p.exists() else None for p in changes}
    written = []
    try:
        # Exclude private material before placing files into the working tree.
        ordered = [exclude_path] + [p for p in changes if p != exclude_path]
        for target in ordered:
            if target not in changes:
                continue
            atomic_write(target, changes[target])
            written.append(target)
    except Exception:
        for target in reversed(written):
            original = backups[target]
            if original is None:
                target.unlink(missing_ok=True)
            else:
                atomic_write(target, original)
        raise
    print("Private configuration written; Qoder parsing, interactive decisions and MCP remain unverified.")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (ValueError, OSError, subprocess.TimeoutExpired, UnicodeError, json.JSONDecodeError) as exc:
        print(f"STOP: {exc}", file=sys.stderr)
        raise SystemExit(2)
