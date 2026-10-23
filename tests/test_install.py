"""Safety checks for private configuration import; no repository scripts are run."""
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest

PACKAGE = Path(__file__).resolve().parents[1]


class PrivateImportTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory(prefix="qoder-import-test-")
        self.root = Path(self.tmp.name)
        self.repo = self.root / "MaaEnd"
        self.config = self.root / "cn"
        self.config.mkdir()
        (self.config / "AGENTS.md").write_text("PRIVATE GLOBAL RULES\n", encoding="utf-8")
        shutil.copytree(PACKAGE / "snapshot/MaaEnd", self.repo)
        self.run_git("init", "-q")
        self.run_git("config", "user.name", "Fixture")
        self.run_git("config", "user.email", "fixture@example.invalid")
        self.run_git("add", ".")
        self.run_git("-c", "core.hooksPath=", "commit", "-qm", "chore: fixture baseline")
        self.original = self.tracked_hashes()

    def tearDown(self):
        self.tmp.cleanup()

    def run_git(self, *args):
        return subprocess.run(["git", "-C", str(self.repo), *args], check=True,
                              capture_output=True, text=True, timeout=15).stdout

    def tracked_hashes(self):
        return {p: hashlib.sha256((self.repo / p).read_bytes()).hexdigest()
                for p in self.run_git("ls-files").splitlines()}

    def install(self, *args, expected=0):
        result = subprocess.run([sys.executable, str(PACKAGE / "install.py"),
                                 "--repo", str(self.repo), "--config-root", str(self.config), *args],
                                capture_output=True, text=True, timeout=45)
        self.assertEqual(result.returncode, expected, result.stdout + result.stderr)
        return result

    def test_plan_does_not_write_and_apply_is_idempotent(self):
        self.install()
        self.assertFalse((self.repo / ".qoder").exists())
        self.install("--apply")
        first = {p.relative_to(self.repo).as_posix(): p.read_bytes()
                 for p in (self.repo / ".qoder").rglob("*") if p.is_file()}
        self.install("--apply")
        second = {p.relative_to(self.repo).as_posix(): p.read_bytes()
                  for p in (self.repo / ".qoder").rglob("*") if p.is_file()}
        self.assertEqual(first, second)
        self.assertEqual(self.original, self.tracked_hashes())
        self.assertEqual(self.run_git("status", "--porcelain"), "")

    def test_dirty_work_is_preserved(self):
        target = self.repo / "AGENTS.md"
        target.write_bytes(target.read_bytes() + b"\nPRIVATE DIRTY WORK\n")
        before = target.read_bytes()
        self.install("--apply")
        self.assertEqual(before, target.read_bytes())
        self.assertIn("AGENTS.md", self.run_git("status", "--porcelain"))

    def test_existing_private_file_is_not_clobbered(self):
        target = self.repo / ".qoder/agents/perlica-maaend.md"
        target.parent.mkdir(parents=True)
        target.write_bytes(b"private custom definition")
        self.install("--apply", expected=2)
        self.assertEqual(target.read_bytes(), b"private custom definition")
        self.assertFalse((self.repo / ".qoder/settings.local.json").exists())

    def test_existing_mcp_conflict_is_not_overwritten(self):
        target = self.repo / ".qoder/settings.local.json"
        target.parent.mkdir(parents=True)
        original = b'{"mcpServers":{"playwright":{"command":"private-wrapper"}}}'
        target.write_bytes(original)
        self.install("--apply", expected=2)
        self.assertEqual(target.read_bytes(), original)

    def test_unrelated_settings_survive_and_servers_remain_disabled(self):
        target = self.repo / ".qoder/settings.local.json"
        target.parent.mkdir(parents=True)
        target.write_text('{"theme":"private","mcpServers":{"other":{"command":"other"}}}', encoding="utf-8")
        self.install("--apply")
        data = json.loads(target.read_bytes())
        self.assertEqual(data["theme"], "private")
        self.assertEqual(data["mcpServers"]["other"], {"command": "other"})
        for name in ["playwright", "chrome-devtools"]:
            self.assertTrue(data["mcpServers"][name]["disabled"])
            self.assertFalse(data["mcpServers"][name]["trust"])

    def test_current_checkout_skills_are_used_and_name_is_normalized(self):
        target = self.repo / ".agents/skills/dmp-analysis/SKILL.md"
        target.write_bytes(target.read_bytes() + b"\nCURRENT CHECKOUT DIFFERENCE\n")
        self.install("--apply")
        installed = self.repo / ".qoder/skills/windows-dmp-analysis/SKILL.md"
        self.assertEqual(installed.read_bytes(), target.read_bytes())
        self.assertFalse((self.repo / ".qoder/skills/dmp-analysis").exists())
        mapping = json.loads((self.repo / ".qoder/maaend-private/skill-map.json").read_bytes())
        self.assertEqual(len(mapping), 24)

    def test_team_roles_commands_and_quality_materials_are_integrated(self):
        self.install("--apply")
        definitions = sorted((self.repo / ".qoder/agents").glob("*.md"))
        self.assertEqual(len(definitions), 7)
        for definition in definitions:
            text = definition.read_text(encoding="utf-8")
            self.assertNotIn("{{SKILLS}}", text)
            self.assertIn("  - maaend-team-workflow\n", text)
            self.assertIn("  - e2e\n", text)
            self.assertIn("  - gdp-ts\n", text)
        self.assertEqual(len(list((self.repo / ".qoder/commands/maaend").glob("*.md"))), 5)
        private = self.repo / ".qoder/maaend-private"
        for path in ["caseflow.py", "Start-Team.ps1", "team-workflow.md", "team-roles.json", "commit-pr-contract.md",
                     "schemas/handoff.schema.json", "quality/gdp/proofs/proof-boundary.ts", "quality/e2e/e2e.config.ts",
                     "research/maaend-workflow-research.md", "research/testing-methods-research.md"]:
            self.assertTrue((private / path).is_file(), path)
        self.assertEqual(self.original, self.tracked_hashes())
        self.assertEqual(self.run_git("status", "--porcelain"), "")

    def test_existing_explicit_mcp_enablement_is_preserved_on_upgrade(self):
        self.install("--apply")
        target = self.repo / ".qoder/settings.local.json"
        settings = json.loads(target.read_bytes())
        settings["mcpServers"]["playwright"]["disabled"] = False
        target.write_text(json.dumps(settings), encoding="utf-8")
        self.install("--apply")
        self.assertFalse(json.loads(target.read_bytes())["mcpServers"]["playwright"]["disabled"])

    def test_user_skill_shadow_blocks_import(self):
        target = self.config / "skills/other-folder/SKILL.md"
        target.parent.mkdir(parents=True)
        target.write_text("---\nname: perlica-style-reply\ndescription: private\n---\nDifferent\n", encoding="utf-8")
        self.install("--apply", expected=2)
        self.assertFalse((self.repo / ".qoder").exists())

    def test_user_agent_shadow_blocks_until_explicitly_allowed(self):
        target = self.config / "agents/perlica-maaend.md"
        target.parent.mkdir(parents=True)
        target.write_text("---\nname: perlica-maaend\n---\nPrivate user-level copy\n", encoding="utf-8")
        # Default: the shadow is a hard stop, so nothing is written.
        self.install("--apply", expected=2)
        self.assertFalse((self.repo / ".qoder").exists())
        # Explicit acknowledgement installs and records the shadow for audit.
        self.install("--apply", "--allow-user-agents")
        self.assertTrue((self.repo / ".qoder/agents/perlica-maaend.md").exists())
        receipt = json.loads((self.repo / ".qoder/maaend-private/install-receipt.json").read_bytes())
        self.assertIn("perlica-maaend", receipt["user_agent_shadow"])
        self.assertEqual(self.original, self.tracked_hashes())

    def test_missing_global_needs_explicit_seed_and_never_replaces_existing(self):
        target = self.config / "AGENTS.md"
        target.unlink()
        self.install(expected=2)
        self.install("--seed-global")
        self.assertFalse(target.exists())
        self.install("--seed-global", "--apply")
        self.assertEqual(target.read_bytes(), (PACKAGE / "policy/global-baseline.md").read_bytes())

    def test_edited_managed_skill_blocks_update(self):
        self.install("--apply")
        target = self.repo / ".qoder/skills/pipeline-guide/SKILL.md"
        target.write_bytes(target.read_bytes() + b"\nLOCAL EDIT\n")
        original = target.read_bytes()
        self.install("--apply", expected=2)
        self.assertEqual(original, target.read_bytes())

    def test_tracked_receipt_is_preserved(self):
        target = self.repo / ".qoder/maaend-private/install-receipt.json"
        target.parent.mkdir(parents=True)
        original = b'{"managed":{},"private_note":"KEEP"}'
        target.write_bytes(original)
        self.run_git("add", str(target))
        self.run_git("-c", "core.hooksPath=", "commit", "-qm", "chore: tracked fixture receipt")
        self.install("--apply", expected=2)
        self.assertEqual(target.read_bytes(), original)
        self.assertEqual(self.run_git("status", "--porcelain"), "")

    def test_edited_receipt_is_preserved(self):
        self.install("--apply")
        target = self.repo / ".qoder/maaend-private/install-receipt.json"
        value = json.loads(target.read_bytes())
        value["private_note"] = "KEEP"
        original = json.dumps(value).encode()
        target.write_bytes(original)
        self.install("--apply", expected=2)
        self.assertEqual(target.read_bytes(), original)

    @unittest.skipUnless(os.name != "nt", "Windows reparse/junction qualification is a separate local check")
    def test_symlink_destination_is_refused(self):
        outside = self.root / "outside"
        outside.mkdir()
        (self.repo / ".qoder").symlink_to(outside, target_is_directory=True)
        self.install("--apply", expected=2)
        self.assertEqual(list(outside.iterdir()), [])


if __name__ == "__main__":
    unittest.main()
