# Performance Benchmarks

This suite keeps compiler and VVP runtime measurements separate. Every timed
simulation must exactly match a checked-in output oracle. The runner performs a
warmup, repeats each phase, and records every sample plus median, mean, range,
median absolute deviation, and population standard deviation in JSON.

Performance commits in this fork use a `[perf] ...` subject. A change is not a
candidate for such a commit until a profile identifies its path as dominant,
the benchmark output is unchanged, and repeated measurements show a gain.

## Workloads

| Name | Primary pressure |
| --- | --- |
| `event_queue` | VVP event scheduling and chained value propagation |
| `vector_arith` | Wide vector multiply, divide, shifts, and real conversion |
| `elaboration` | Parameterized instances, generated scopes, nets, and expressions |
| `event_queue_heavy` | Event scheduling scaled for low-noise paired timing |
| `vector_arith_heavy` | Wide arithmetic scaled for low-noise paired timing |
| `elaboration_heavy` | Elaboration scaled for low-noise paired timing |
| `picorv32` | PicoRV32 running its ISA-test firmware from a pinned external corpus |

The synthetic workloads are deliberately deterministic and long enough to
measure without producing large output files. They are not substitutes for
real designs; external design snapshots are added as separate benchmark
families once their source and output provenance are pinned.

## Running

Use a clean, optimized, non-sanitized build. Point the runner at the installed
in-tree binaries so the driver loads modules from the same build:

```sh
python3 benchmarks/run.py \
  --iverilog build/perf-release/install/bin/iverilog \
  --label master-baseline
```

`--vvp` defaults to the binary beside `--iverilog`. Select individual workloads
with repeated `--workload NAME`, change repetition with `--runs` and
`--warmups`, or isolate one phase with `--phase compile` or
`--phase simulate`. Generated files and reports live under
`benchmarks/.work/`, which is ignored by Git.

## Comparing Changes

Run the same suite before and after a proposed optimization, preferably in
alternating batches on an otherwise idle machine. Compare the reports with:

```sh
python3 benchmarks/compare.py \
  benchmarks/.work/results/BASELINE.json \
  benchmarks/.work/results/CANDIDATE.json
```

The comparison refuses reports whose workload source hashes or output-oracle
hashes differ. Keep compiler flags, hardware power mode, workload selection,
and run counts identical. Treat very short phases and changes smaller than the
observed run-to-run dispersion as inconclusive.

For a direct same-host comparison, `paired.py` pins both tools to one logical
CPU and alternates them in balanced ABBA/BAAB blocks. Each simulation is
checked against its oracle, and the JSON retains every wall-time and CPU-time
sample:

```sh
python3 benchmarks/paired.py \
  --baseline-iverilog /path/to/baseline/bin/iverilog \
  --baseline-label baseline \
  --candidate-iverilog /path/to/candidate/bin/iverilog \
  --candidate-label candidate \
  --workload vector_arith_heavy \
  --phase simulate \
  --cpu 3 --blocks 5 --warmups 1 \
  --work-dir benchmarks/.work/paired \
  --output benchmarks/.work/paired.json
```

Use the geometric mean of the per-block ratios as the primary paired result,
and inspect its range and wall/CPU agreement before drawing a conclusion.

## Profile-First Protocol

1. Reproduce the baseline and retain its JSON report.
2. Profile compile and simulation as different processes. Do not infer one
   phase's hot paths from the other.
3. Optimize only a dominant stack or allocation site visible in the selected
   workload profile.
4. Rebuild, run the full regression suite, and rerun this benchmark suite.
5. Require exact simulation output and repeated timing improvement before
   keeping a `[perf]` commit.

On Linux, `perf record -g -- iverilog ...` and `perf record -g -- vvp ...`
provide the first profiles. On macOS, use Instruments' Time Profiler or launch
the process and attach `/usr/bin/sample`. Store raw profiles outside Git; put
only the commands, dominant stacks, timing reports, and conclusions in local
campaign notes.
