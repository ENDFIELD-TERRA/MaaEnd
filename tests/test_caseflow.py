"""Integrated private handoff checks. These fixtures do not represent MaaEnd behavior."""
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import unittest

import test_install as importer


class CaseflowIntegrationTests(unittest.TestCase):
    setUp = importer.PrivateImportTests.setUp
    tearDown = importer.PrivateImportTests.tearDown
    run_git = importer.PrivateImportTests.run_git
    tracked_hashes = importer.PrivateImportTests.tracked_hashes
    install = importer.PrivateImportTests.install
    def run_flow(self, *args, expected=0):
        result = subprocess.run([sys.executable, str(self.repo / ".qoder/maaend-private/caseflow.py"),
                                 "--repo", str(self.repo), *args], capture_output=True, text=True, timeout=20)
        self.assertEqual(result.returncode, expected, result.stdout + result.stderr)
        return result

    def prepare_case(self):
        self.install("--apply")
        event = json.loads((PACKAGE / "examples/issue-event.json").read_bytes())
        self.event = self.root / "event.json"
        self.event.write_text(json.dumps(event), encoding="utf-8")
        self.run_flow("intake", "--event", str(self.event))
        self.case_id = "maaend-maaend-6483"
        self.folder = self.repo / ".qoder/maaend-private/cases" / self.case_id
        self.product = self.repo / "local-integration-fixture.txt"
        self.product.write_bytes(b"Private integration fixture; not a game or test target.\n")
        self.manifest = self.root / "candidate.json"
        self.manifest.write_text(json.dumps({"build_id": "fixture-build", "files": {
            self.product.name: hashlib.sha256(self.product.read_bytes()).hexdigest()}}), encoding="utf-8")
        self.run_flow("--case", self.case_id, "freeze", "--manifest", str(self.manifest))
        self.state = json.loads((self.folder / "case.json").read_bytes())

    def report(self, phase, author, expected=0, changes=None):
        artifact = self.folder / (phase + ".md")
        artifact.write_text("Private plumbing fixture. No product acceptance is claimed.\n", encoding="utf-8")
        report = {"phase": phase, "generation": self.state["generation"], "candidate_id": self.state["candidate"]["id"],
                  "author": author, "status": "passed", "summary": "Private record plumbing only",
                  "artifacts": [{"path": artifact.name, "sha256": hashlib.sha256(artifact.read_bytes()).hexdigest()}]}
        report.update(changes or {})
        source = self.root / (phase + ".json")
        source.write_text(json.dumps(report), encoding="utf-8")
        return self.run_flow("--case", self.case_id, "record", "--report", str(source), expected=expected)

    def test_full_private_flow_and_downstream_invalidation(self):
        self.prepare_case()
        self.report("integration", "builder")
        self.report("review", "reviewer")
        self.report("validation", "validator")
        status = json.loads(self.run_flow("--case", self.case_id, "status").stdout)
        self.assertTrue(status["ready_for_human_review"])
        self.assertFalse(status["public_write"])
        self.product.write_bytes(b"changed candidate\n")
        self.run_flow("--case", self.case_id, "status", expected=2)

    def test_replayed_events_do_not_create_a_new_generation(self):
        self.prepare_case()
        reply = json.loads(self.run_flow("intake", "--event", str(self.event)).stdout)
        self.assertTrue(reply["deduplicated"])
        self.assertEqual(reply["generation"], 1)

    def test_validation_cannot_precede_integration_or_independent_review(self):
        self.prepare_case()
        self.report("validation", "validator", expected=2)
        self.report("integration", "builder")
        self.report("review", "builder", expected=2)

    def test_malformed_reports_never_enter_the_acceptance_record(self):
        self.prepare_case()
        for changes in [{"generation": True}, {"author": ["builder"]}, {"summary": {"text": "ok"}},
                        {"summary": "  "}, {"candidate_id": "invalid"}, {"unexpected": "field"},
                        {"artifacts": [{"path": "integration.md", "sha256": "invalid"}]},
                        {"artifacts": [{"path": "integration.md", "sha256": "0" * 64, "extra": True}]}]:
            with self.subTest(changes=changes):
                self.report("integration", "builder", expected=2, changes=changes)
                self.assertEqual(json.loads((self.folder / "case.json").read_bytes())["reports"], {})

    def test_changed_evidence_is_not_reused_as_a_passing_result(self):
        self.prepare_case()
        self.report("integration", "builder")
        self.report("review", "reviewer")
        (self.folder / "integration.md").write_bytes(b"edited evidence")
        self.report("validation", "validator", expected=2)

    def test_bot_mentions_are_skipped_and_new_human_evidence_advances_generation(self):
        self.prepare_case()
        event = json.loads(self.event.read_bytes())
        event["event_name"] = "issue_comment"
        event["payload"]["action"] = "created"
        event["payload"]["comment"] = {"id": 42, "body": "@MaaEndBot new evidence", "updated_at": "2026-10-21T01:00:00Z", "user": {"type": "Bot"}}
        self.event.write_text(json.dumps(event), encoding="utf-8")
        skipped = json.loads(self.run_flow("intake", "--event", str(self.event)).stdout)
        self.assertFalse(skipped["accepted"])
        event["payload"]["comment"]["user"]["type"] = "User"
        self.event.write_text(json.dumps(event), encoding="utf-8")
        reply = json.loads(self.run_flow("intake", "--event", str(self.event)).stdout)
        self.assertEqual(reply["generation"], 2)
        state = json.loads((self.folder / "case.json").read_bytes())
        self.assertEqual(state["reports"], {})


PACKAGE = Path(__file__).resolve().parents[1]


if __name__ == "__main__":
    unittest.main()
