#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Bounded calibrated Preview ASan/UBSan checks; never original binary replay."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import platform
import re
import shlex
import shutil
import subprocess

_spec = importlib.util.spec_from_file_location("k1_evidence",
    Path(__file__).with_name("replay-k1-original.py"))
_evidence = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(_evidence)
REQUIRED_FLAGS = ("-fsanitize=address,undefined", "-fno-sanitize-recover=all",
                  "-fno-omit-frame-pointer", "-O1")


def digest(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def classify_control(name, code, text):
    if code == 0:
        raise ValueError("Broken sanitizer control unexpectedly succeeded")
    required = {"asan": r"ERROR: AddressSanitizer: heap-buffer-overflow",
                "ubsan": r"runtime error: signed integer overflow"}[name]
    if not re.search(required, text):
        raise ValueError("Control failed without the expected sanitizer report")


def run_campaign(source, build, controls, revision, output):
    source,build,controls,output = [Path(p).resolve() for p in (source,build,controls,output)]
    if output.exists():
        raise ValueError("Existing sanitizer evidence is never overwritten")
    output.mkdir(parents=True,exist_ok=False)
    executable=build/"sawstar_premium_native_pair"
    manifest=dict(schema=1,state="incomplete",revision=revision,
        instrumented=True,rebuild=True,original_binary_replay=False,fp_override=False,
        required_compile_flags=list(REQUIRED_FLAGS),leak_detection=False,
        purpose="Calibrated Preview memory/UB checks; not K1 closure, CPU or native host acceptance",
        production_promotion_allowed=False,native_host_acceptance=False,results=[])

    def save():
        (output/"manifest.json").write_text(json.dumps(manifest,indent=2)+"\n",encoding="utf-8")

    save()
    try:
        if platform.system()!="Darwin" or platform.machine() not in ("arm64","x86_64"):
            raise ValueError("Expected native macOS arm64 or Intel runner")
        manifest.update(host_architecture=platform.machine(),
            os_version=subprocess.check_output(["sw_vers"],text=True),
            runner_image=os.environ.get("ImageVersion"),
            compiler=subprocess.check_output(["clang++","--version"],text=True),
            runner_source_commit=subprocess.check_output(["git","rev-parse","HEAD"],text=True).strip(),
            tested_source_commit=subprocess.check_output(["git","-C",str(source),"rev-parse","HEAD"],text=True).strip(),
            asan_options=os.environ.get("ASAN_OPTIONS"),ubsan_options=os.environ.get("UBSAN_OPTIONS"))
        expected_source=_evidence.SOURCE if revision=="original" else manifest["runner_source_commit"]
        if manifest["tested_source_commit"]!=expected_source:
            raise ValueError("Tested source differs from the declared revision")
        translated=subprocess.run(["sysctl","-in","sysctl.proc_translated"],capture_output=True,text=True)
        manifest["translation_probe"]=dict(returncode=translated.returncode,stdout=translated.stdout.strip())
        if translated.returncode==0 and translated.stdout.strip() not in ("","0"):
            raise ValueError("Sanitizer jobs must run natively, without Rosetta")
        commands=json.loads((build/"compile_commands.json").read_text())
        if not commands:
            raise ValueError("Missing instrumented compilation database")
        for entry in commands:
            args=entry.get("arguments") or shlex.split(entry["command"])
            if any(flag not in args for flag in REQUIRED_FLAGS):
                raise ValueError("Uninstrumented translation unit: "+entry["file"])
        manifest["instrumented_translation_units"]=len(commands)
        symbols=subprocess.check_output(["nm",str(executable)],text=True,stderr=subprocess.STDOUT)
        if "__asan_init" not in symbols or "__ubsan_handle_" not in symbols:
            raise ValueError("Engine executable lacks required sanitizer runtime references")
        (output/"sanitizer-symbols.txt").write_text("\n".join(
            line for line in symbols.splitlines() if "__asan_" in line or "__ubsan_" in line)+"\n")
        for src,name in [(executable,"sawstar_premium_native_pair"),(controls,"sanitizer-controls"),
                         (build/"CMakeCache.txt","CMakeCache.txt"),
                         (build/"compile_commands.json","compile_commands.json")]:
            shutil.copy2(src,output/name)
        manifest.update(executable_sha256=digest(executable),controls_sha256=digest(controls),
            cache_sha256=digest(build/"CMakeCache.txt"),
            compile_commands_sha256=digest(build/"compile_commands.json"))
        save()
        for kind in ("asan","ubsan"):
            log=output/("control-"+kind+".txt")
            entry=dict(name="control-"+kind,expected_failure=True)
            with log.open("w",encoding="utf-8") as stream:
                try:
                    result=subprocess.run([str(controls),"--"+kind],stdout=stream,
                        stderr=subprocess.STDOUT,timeout=30)
                    entry["returncode"]=result.returncode
                except subprocess.TimeoutExpired:
                    entry.update(returncode=124,validation_error="Sanitizer control timeout")
            entry["log_sha256"]=digest(log)
            if not entry.get("validation_error"):
                try:
                    classify_control(kind,entry["returncode"],log.read_text())
                    entry["expected_report_verified"]=True
                except ValueError as error:
                    entry["validation_error"]=str(error)
            manifest["results"].append(entry);save()
        # Do not qualify real engine runs when the diagnostic controls failed.
        if any(r.get("validation_error") for r in manifest["results"]):
            raise ValueError("Sanitizer runtime controls failed")
        for wave in range(4):
            diagnostic=revision=="original" and wave==2
            args=["--diagnostic-waveform" if diagnostic else "--waveform",str(wave)]
            log=output/("waveform-"+str(wave)+".txt")
            entry=dict(name="waveform-"+str(wave),arguments=args,expected_failure=False)
            with log.open("w",encoding="utf-8") as stream:
                try:
                    result=subprocess.run([str(executable)]+args,stdout=stream,
                        stderr=subprocess.STDOUT,timeout=300)
                    entry["returncode"]=result.returncode
                except subprocess.TimeoutExpired:
                    entry.update(returncode=124,timeout=True)
            entry["log_sha256"]=digest(log)
            text=log.read_text(encoding="utf-8",errors="replace")
            entry["sanitizer_reported"]=bool(re.search(
                r"ERROR: AddressSanitizer|runtime error:|SUMMARY: (?:AddressSanitizer|UndefinedBehaviorSanitizer)",text))
            entry["numerical_divergence_reported"]="Dispatch engine bit identity waveform=" in text
            if not entry["returncode"]:
                try:
                    fingerprint=_evidence.validate_table(text,wave,diagnostic=diagnostic)
                    entry["validated_cases"]=120
                    if fingerprint:
                        entry["fingerprint_table_sha256"]=fingerprint
                except ValueError as error:
                    entry["validation_error"]=str(error)
            manifest["results"].append(entry);save()
            print(revision,platform.machine(),"waveform",wave,"exit",entry["returncode"],
                entry.get("validation_error",""),"sanitizer_reported",entry["sanitizer_reported"],flush=True)
        if (digest(executable)!=manifest["executable_sha256"] or
            digest(build/"CMakeCache.txt")!=manifest["cache_sha256"] or
            digest(controls)!=manifest["controls_sha256"]):
            raise ValueError("Build artifacts changed during the campaign")
        failed=any(r.get("validation_error") or
            (not r["expected_failure"] and (r["returncode"] or r["sanitizer_reported"])) for r in manifest["results"])
        manifest["state"]="failed" if failed else "complete"
    except BaseException as error:
        manifest.update(state="failed",error=str(error))
        raise
    finally:
        save()
    return manifest


if __name__=="__main__":
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source",required=True,type=Path)
    parser.add_argument("--build",required=True,type=Path)
    parser.add_argument("--controls",required=True,type=Path)
    parser.add_argument("--revision",required=True,choices=("original","current"))
    parser.add_argument("--output",required=True,type=Path)
    args=parser.parse_args()
    result=run_campaign(args.source,args.build,args.controls,args.revision,args.output)
    if result["state"]!="complete":
        parser.exit(1,"Calibrated sanitizer campaign failed; inspect retained evidence\n")
