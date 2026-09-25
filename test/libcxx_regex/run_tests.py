#!/usr/bin/env python3
"""Compile and run the vendored libc++ port without third-party test dependencies."""

import argparse
import concurrent.futures
import json
import os
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cxx", default=os.environ.get("CXX", "c++"))
    parser.add_argument("--build-dir", required=True, type=Path)
    parser.add_argument("--jobs", type=int, default=4)
    parser.add_argument("--filter", default="")
    parser.add_argument("--kind", choices=("pass", "compile_fail"))
    parser.add_argument("--flags", default="")
    parser.add_argument("--run-only", action="store_true")
    args = parser.parse_args()
    args.build_dir.mkdir(parents=True, exist_ok=True)
    manifest = json.loads((ROOT / "manifest.json").read_text())
    tests = [
        t
        for t in manifest["tests"]
        if t["status"] in ("pass", "compile_fail")
        and args.filter in t["source"]
        and (args.kind is None or args.kind == t["status"])
    ]
    if not tests:
        parser.error("no tests selected")

    def run(test):
        import shlex

        name = test["source"]
        output = args.build_dir / name.replace("/", "_")
        log = output.with_suffix(".log")
        command = [
            args.cxx,
            "-std=c++20",
            "-O0",
            "-g",
            "-UNDEBUG",
            "-fexceptions",
            "-I" + str(ROOT.parents[1] / "include"),
            "-I" + str(ROOT),
            "-I" + str(ROOT / "support"),
            *shlex.split(args.flags),
            str(ROOT / "tests" / name),
        ]
        negative = test["status"] == "compile_fail"
        command += ["-fsyntax-only"] if negative else ["-o", str(output)]
        try:
            if not args.run_only or negative:
                build = subprocess.run(
                    command, capture_output=True, text=True, timeout=120
                )
                log.write_text(build.stdout + build.stderr)
                if negative:
                    # Check every annotated constructor, not merely that one of
                    # several expressions in a verify test fails to compile.
                    errors = [
                        line for line in build.stderr.splitlines() if "error:" in line
                    ]
                    valid = (
                        build.returncode != 0
                        and errors
                        and all("deleted" in line for line in errors)
                    )
                    source_lines = (ROOT / "tests" / name).read_text().splitlines()
                    for line_number, line in enumerate(source_lines, start=1):
                        annotation = re.search(r"expected-error@(-\d+)", line)
                        if annotation:
                            start = line_number + int(annotation[1])
                            valid = valid and any(
                                any(
                                    f":{number}:" in error
                                    for number in range(start, line_number)
                                )
                                for error in errors
                            )
                    return name, bool(valid), "compile-fail"
                if build.returncode:
                    return name, False, "compile"
            result = subprocess.run(
                [str(output)], capture_output=True, text=True, timeout=30
            )
            log.write_text(result.stdout + result.stderr)
            return name, result.returncode == 0, "run"
        except subprocess.TimeoutExpired:
            return name, False, "timeout"

    results = []
    with concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as executor:
        for name, ok, phase in executor.map(run, tests):
            results.append(ok)
            print(f"{'PASS' if ok else 'FAIL'} [{phase}] {name}", flush=True)
    print(f"{sum(results)}/{len(results)} tests passed; logs: {args.build_dir}")
    return 0 if all(results) else 1


if __name__ == "__main__":
    sys.exit(main())
