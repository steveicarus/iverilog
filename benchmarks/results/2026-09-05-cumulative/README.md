# Cumulative performance refresh: 2026-09-05

This refresh measures the fork directly against current upstream rather than
multiplying speedups quoted by individual commits.

## Results

The primary practical comparison is upstream's stock build against the fork's
recommended tuned build:

| workload and phase | upstream stock median | tuned fork median | paired wall speedup | block range |
| --- | ---: | ---: | ---: | ---: |
| `elaboration_heavy` compile | 13.866 s | 1.000 s | **13.86x** | 13.76-13.94x |
| `vector_arith_heavy` simulate | 14.754 s | 2.051 s | **7.20x** | 7.15-7.27x |
| `event_queue_heavy` simulate | 23.511 s | 5.676 s | **4.14x** | 4.11-4.15x |
| `picorv32` simulate | 11.770 s | 3.808 s | **3.10x** | 3.08-3.14x |

The speedup is the geometric mean of five per-block ratios. Each block contains
two runs of each tool, so every cell above represents ten timed runs per tool.
The fork won all 20 practical-comparison blocks. The unweighted geometric mean
of the four headline speedups is 5.98x, but it is not a suite-throughput claim:
the workloads exercise different phases and use intentionally different scale
factors.

Matched-build comparisons separate source changes from the build recipe:

| workload and phase | GCC 13 `-O2` vs GCC 13 `-O2` | tuned vs tuned | stock upstream vs tuned fork |
| --- | ---: | ---: | ---: |
| `elaboration_heavy` compile | 12.39x | 14.63x | **13.86x** |
| `vector_arith_heavy` simulate | 5.52x | 5.83x | **7.20x** |
| `event_queue_heavy` simulate | 3.47x | 3.37x | **4.14x** |
| `picorv32` simulate | 2.36x | 2.59x | **3.10x** |

All 60 blocks across the three comparison modes favored the fork. CPU-time
speedups closely match wall-time speedups; the exact figures and every sample
are retained in the JSON files beside this report.

## Revisions

- Upstream: `steveicarus/iverilog` `5ab23063fe15bae91f8453e5f50a35cb03ea3206`
- Fork: `apullin/aiverilog` `b7c94d088ee2b52a35c7bd80f8cf8a7927f66cee`
- Shared merge base: `64f13540a6ec8122c16b10efa220ed0eff4686f9`

Both remotes were fetched immediately before measurement. This is a
current-tip versus current-tip comparison: upstream has 12 commits after the
shared base that are not ancestors of the fork, while the fork has 254 commits
after that base that are not ancestors of upstream.

The tuned fork artifact was built from pre-merge commit `5c02377ef`; its Git
tree (`3a5bfe1cb74dfbd40aa89c2145926d80f865c1aa`) is byte-for-byte the same tree
as merged fork revision `b7c94d088`, so the reports identify the merged
revision.

## Builds

- Stock upstream and fork: GCC 13.3.0, Autoconf defaults, `-g -O2`.
- Tuned upstream and fork: Clang 20.1.2, `-O3 -g -fomit-frame-pointer
  -flto=thin`, then `-fprofile-use`.
- Each tuned tree was independently instrumented and trained with one complete
  compile-and-simulate pass over the same seven oracle-checked workloads before
  its final build. The training source and oracle hashes match exactly.

Artifact hashes are embedded in every raw report. A given label resolves to a
single `iverilog`/`vvp` hash pair across all workloads.

## Method

- Host: `para`, AMD Ryzen Threadripper PRO 7975WX (32 cores, 64 threads), Linux
  6.17.0-35-generic.
- Affinity: logical CPU 31 throughout; its SMT sibling is CPU 63.
- Frequency policy: `amd-pstate-epp`, `powersave` governor,
  `balance_performance` energy preference, boost enabled.
- One warmup per tool, followed by five alternating ABBA/BAAB blocks.
- Two timed executions per tool in every block; all work ran serially.
- Every generated program was validated with its matching simulator before
  timing. Every timed simulation matched the checked-in stdout oracle exactly
  and emitted no stderr.
- Wall time used `CLOCK_MONOTONIC`; process CPU time used `RUSAGE_CHILDREN`.
- The primary statistic is the geometric mean of the five block ratios. Raw
  reports also include medians, means, minima, maxima, MAD, population standard
  deviation, timestamps, commands' output hashes, tool hashes, and source
  hashes.

The runner used for these measurements is [`../../paired.py`](../../paired.py).
The twelve `*.json` files in this directory are the complete raw results.
