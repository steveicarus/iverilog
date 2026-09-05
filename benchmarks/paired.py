#!/usr/bin/env python3

"""Run a same-core, alternating comparison of two Icarus installations."""

import argparse
import datetime
import hashlib
import json
import os
from pathlib import Path
import platform
import resource
import statistics
import subprocess
import sys
import time

from run import WORKLOADS, digest_file, resolve_tool, source_label, tool_metadata


def timed_run(command, *, cwd, env):
    usage_before = resource.getrusage(resource.RUSAGE_CHILDREN)
    started = datetime.datetime.now(datetime.timezone.utc)
    start_ns = time.perf_counter_ns()
    result = subprocess.run(
        command,
        cwd=cwd,
        env=env,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=False,
    )
    wall = (time.perf_counter_ns() - start_ns) / 1_000_000_000
    usage_after = resource.getrusage(resource.RUSAGE_CHILDREN)
    cpu = (
        usage_after.ru_utime
        - usage_before.ru_utime
        + usage_after.ru_stime
        - usage_before.ru_stime
    )
    if result.returncode:
        sys.stderr.buffer.write(result.stdout)
        sys.stderr.buffer.write(result.stderr)
        raise RuntimeError(
            f"command exited {result.returncode}: {' '.join(map(str, command))}"
        )
    return {
        "started_utc": started.isoformat(),
        "wall_seconds": wall,
        "cpu_seconds": cpu,
        "stdout_sha256": hashlib.sha256(result.stdout).hexdigest(),
        "stderr_sha256": hashlib.sha256(result.stderr).hexdigest(),
    }


def summary(values):
    median = statistics.median(values)
    return {
        "samples": values,
        "minimum": min(values),
        "median": median,
        "mean": statistics.mean(values),
        "maximum": max(values),
        "mad": statistics.median(abs(value - median) for value in values),
        "pstdev": statistics.pstdev(values),
    }


def tool_pair(iverilog_arg, vvp_arg):
    iverilog = resolve_tool(iverilog_arg)
    if vvp_arg:
        vvp = resolve_tool(vvp_arg)
    else:
        sibling = iverilog.with_name("vvp")
        vvp = sibling if sibling.is_file() else resolve_tool("vvp")
    return iverilog, vvp


def compile_command(iverilog, workload, output):
    sources = [
        str(path.relative_to(workload.cwd)) if workload.cwd else str(path)
        for path in workload.sources
    ]
    return [
        str(iverilog),
        "-g2012",
        *workload.iverilog_args,
        "-s",
        workload.top,
        "-o",
        str(output),
        *sources,
    ]


def validate_simulation(workload, result):
    expected = digest_file(workload.gold)
    if result["stdout_sha256"] != expected:
        raise RuntimeError(f"{workload.name} stdout does not match its oracle")
    empty = hashlib.sha256(b"").hexdigest()
    if result["stderr_sha256"] != empty:
        raise RuntimeError(f"{workload.name} wrote unexpected stderr")


def parse_args():
    parser = argparse.ArgumentParser(
        description="Compare two Icarus installations in balanced ABBA blocks."
    )
    parser.add_argument("--baseline-iverilog", required=True)
    parser.add_argument("--baseline-vvp")
    parser.add_argument("--baseline-label", default="baseline")
    parser.add_argument("--baseline-commit")
    parser.add_argument("--candidate-iverilog", required=True)
    parser.add_argument("--candidate-vvp")
    parser.add_argument("--candidate-label", default="candidate")
    parser.add_argument("--candidate-commit")
    parser.add_argument("--workload", required=True, choices=sorted(WORKLOADS))
    parser.add_argument("--phase", required=True, choices=("compile", "simulate"))
    parser.add_argument("--cpu", type=int, required=True)
    parser.add_argument("--blocks", type=int, default=5)
    parser.add_argument("--warmups", type=int, default=1)
    parser.add_argument("--work-dir", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    return parser.parse_args()


def main():
    args = parse_args()
    if args.blocks < 1 or args.warmups < 0:
        raise RuntimeError("--blocks must be positive and --warmups nonnegative")
    os.sched_setaffinity(0, {args.cpu})

    workload = WORKLOADS[args.workload]
    if not workload.staged():
        raise RuntimeError(f"workload {workload.name} is not staged")

    baseline_iverilog, baseline_vvp = tool_pair(
        args.baseline_iverilog, args.baseline_vvp
    )
    candidate_iverilog, candidate_vvp = tool_pair(
        args.candidate_iverilog, args.candidate_vvp
    )
    tools = {
        "baseline": (baseline_iverilog, baseline_vvp),
        "candidate": (candidate_iverilog, candidate_vvp),
    }

    work_dir = args.work_dir.resolve()
    work_dir.mkdir(parents=True, exist_ok=True)
    programs = {
        name: work_dir / f"{workload.name}-{name}.vvp" for name in tools
    }
    commands = {
        name: compile_command(pair[0], workload, programs[name])
        for name, pair in tools.items()
    }
    env = os.environ.copy()
    env.update({"LC_ALL": "C", "LANG": "C", "TZ": "UTC"})

    # Validate a version-specific executable before timing either phase.
    for name, pair in tools.items():
        compile_result = timed_run(commands[name], cwd=workload.cwd, env=env)
        empty = hashlib.sha256(b"").hexdigest()
        if (
            compile_result["stdout_sha256"] != empty
            or compile_result["stderr_sha256"] != empty
        ):
            raise RuntimeError(
                f"{workload.name} compile emitted diagnostics for {name}"
            )
        simulation = timed_run(
            [str(pair[1]), str(programs[name]), *workload.vvp_args],
            cwd=workload.cwd,
            env=env,
        )
        validate_simulation(workload, simulation)
        if args.phase == "simulate":
            commands[name] = [str(pair[1]), str(programs[name]), *workload.vvp_args]

    expected_hashes = None
    for _ in range(args.warmups):
        for name in ("baseline", "candidate"):
            result = timed_run(commands[name], cwd=workload.cwd, env=env)
            if args.phase == "simulate":
                validate_simulation(workload, result)
            hashes = (result["stdout_sha256"], result["stderr_sha256"])
            if expected_hashes is None:
                expected_hashes = hashes
            elif hashes != expected_hashes:
                raise RuntimeError(f"output mismatch during warmup: {name}")

    samples = {
        name: {"wall_seconds": [], "cpu_seconds": []} for name in tools
    }
    ratios = {"wall": [], "cpu": []}
    raw_blocks = []
    for block in range(args.blocks):
        order = (
            ("baseline", "candidate", "candidate", "baseline")
            if block % 2 == 0
            else ("candidate", "baseline", "baseline", "candidate")
        )
        block_samples = {"baseline": [], "candidate": []}
        for name in order:
            result = timed_run(commands[name], cwd=workload.cwd, env=env)
            if args.phase == "simulate":
                validate_simulation(workload, result)
            hashes = (result["stdout_sha256"], result["stderr_sha256"])
            if expected_hashes is None:
                expected_hashes = hashes
            elif hashes != expected_hashes:
                raise RuntimeError(f"output mismatch in block {block}: {name}")
            block_samples[name].append(result)
            samples[name]["wall_seconds"].append(result["wall_seconds"])
            samples[name]["cpu_seconds"].append(result["cpu_seconds"])

        block_ratio = {}
        for short, field in (("wall", "wall_seconds"), ("cpu", "cpu_seconds")):
            baseline_mean = statistics.mean(
                result[field] for result in block_samples["baseline"]
            )
            candidate_mean = statistics.mean(
                result[field] for result in block_samples["candidate"]
            )
            block_ratio[short] = baseline_mean / candidate_mean
            ratios[short].append(block_ratio[short])
        raw_blocks.append(
            {
                "index": block,
                "order": order,
                "samples": block_samples,
                "speedup": block_ratio,
            }
        )
        print(
            f"block {block + 1}/{args.blocks}: "
            f"wall {block_ratio['wall']:.4f}x, cpu {block_ratio['cpu']:.4f}x",
            flush=True,
        )

    report = {
        "schema": 1,
        "timestamp_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
        "hostname": platform.node(),
        "platform": platform.platform(),
        "python": platform.python_version(),
        "cpu": args.cpu,
        "blocks": args.blocks,
        "warmups": args.warmups,
        "runs_per_tool": 2 * args.blocks,
        "phase": args.phase,
        "workload": {
            "name": workload.name,
            "description": workload.description,
            "sources": {
                source_label(path): digest_file(path)
                for path in (*workload.sources, *workload.inputs)
            },
            "oracle_sha256": digest_file(workload.gold),
        },
        "tools": {
            "baseline": {
                "label": args.baseline_label,
                "commit": args.baseline_commit,
                "iverilog": tool_metadata(baseline_iverilog, "-V"),
                "vvp": tool_metadata(baseline_vvp, "-V"),
            },
            "candidate": {
                "label": args.candidate_label,
                "commit": args.candidate_commit,
                "iverilog": tool_metadata(candidate_iverilog, "-V"),
                "vvp": tool_metadata(candidate_vvp, "-V"),
            },
        },
        "samples": {
            name: {metric: summary(values) for metric, values in metrics.items()}
            for name, metrics in samples.items()
        },
        "speedup": {
            metric: {
                **summary(values),
                "geomean": statistics.geometric_mean(values),
                "candidate_wins": sum(value > 1 for value in values),
            }
            for metric, values in ratios.items()
        },
        "raw_blocks": raw_blocks,
    }
    args.output.resolve().parent.mkdir(parents=True, exist_ok=True)
    args.output.resolve().write_text(json.dumps(report, indent=2) + "\n")
    print(json.dumps(report["speedup"], indent=2))
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (OSError, RuntimeError, subprocess.CalledProcessError) as error:
        print(f"paired.py: {error}", file=sys.stderr)
        sys.exit(1)
