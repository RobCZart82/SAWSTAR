# SPDX-License-Identifier: MIT
import contextlib
import importlib.util
import io
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

spec = importlib.util.spec_from_file_location("control", Path(__file__).resolve().parents[1] / "scripts/control-audio-timing.py")
control = importlib.util.module_from_spec(spec)
spec.loader.exec_module(control)


def report(cpu=2, peak=-3):
    return "scene,voices,realtime_percent,peak_dbfs,rms_dbfs,dc\n" + "".join(
        f"{s},{v},{cpu},{peak},-12,0\n" for s in ("dry_fx", "full_fx") for v in (1, 8, 16))


class ControlTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.binary = self.root / "benchmark"
        self.binary.write_bytes(b"fixed control executable")
        self.output = self.root / "evidence"

    def run_control(self, outputs):
        with patch.object(control.benchmark, "run", side_effect=outputs) as run, \
                contextlib.redirect_stdout(io.StringIO()):
            result = control.run_control(self.binary, self.output)
        self.assertEqual(run.call_count, 18)
        self.assertEqual({c.args for c in run.call_args_list}, {(str(self.binary.resolve()),)})
        return result

    def test_identical_binary_control_preserves_every_report_and_is_not_acceptance(self):
        result = self.run_control([report()] * 18)
        self.assertFalse(result["production_regression_accepted"])
        self.assertFalse(result["native_cpu_accepted"])
        self.assertEqual(len(result["scenes"]), 6)
        self.assertFalse(any(s["threshold_exceeded"] for s in result["scenes"]))
        manifest = json.loads((self.output / "manifest.json").read_text())
        self.assertEqual(len(manifest["records"]), 18)
        self.assertEqual(manifest["status"], "success")
        with self.assertRaises(FileExistsError):
            control.run_control(self.binary, self.output)

    def test_control_failure_is_not_retried_or_filtered(self):
        values = [report(), report()]
        for i in range(8):
            values.extend([report(2), report(4)] if i % 2 == 0 else [report(4), report(2)])
        result = self.run_control(values)
        self.assertTrue(all(s["threshold_exceeded"] for s in result["scenes"]))
        self.assertEqual(json.loads((self.output / "manifest.json").read_text())["status"], "failed")
        self.assertEqual(json.loads((self.output / "control-result.json").read_text()), result)

    def test_changed_audio_rejects_control(self):
        values = [report()] * 18
        values[-1] = report(peak=-4)
        with self.assertRaisesRegex(RuntimeError, "audio differs"):
            self.run_control(values)
        manifest = json.loads((self.output / "manifest.json").read_text())
        self.assertEqual(manifest["status"], "failed")
        self.assertEqual(len(manifest["records"]), 18)

    def test_replaced_executable_rejects_control(self):
        calls = 0
        def fake_run(*args):
            nonlocal calls
            calls += 1
            if calls == 18:
                self.binary.write_bytes(b"changed executable")
            return report()
        with patch.object(control.benchmark, "run", side_effect=fake_run), \
                contextlib.redirect_stdout(io.StringIO()), \
                self.assertRaisesRegex(RuntimeError, "executable changed"):
            control.run_control(self.binary, self.output)
        self.assertEqual(calls, 18)
        self.assertEqual(json.loads((self.output / "manifest.json").read_text())["status"], "failed")


if __name__ == "__main__":
    unittest.main()
