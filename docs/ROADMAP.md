# Feature roadmap (one-by-one)

Each item should be a dedicated `feat/<name>` branch, with tests/examples and a STATUS.md update. Merge only into this fork’s default branch (`master`).

## Tier A — Compiler SV foundations (block Accellera UVM)

1. **Parameterized classes** — `class C #(type T = int);` / `C#(byte)` — needed for `uvm_*#(T)`, `config_db` — **partial** (defaults; see STATUS)
2. **Associative arrays** — `int aa[string];` — **in progress / partial** (string keys only; see [assoc-array.md](assoc-array.md))
3. **Virtual interfaces** + eventing on `vif.clk` — **partial** (see [virtual-interface.md](virtual-interface.md))
4. **Clocking blocks** — enough for `@(vif.cb)` — **partial** (interface-local `@(bif.cb)`; see [clocking.md](clocking.md))
5. **`mailbox` / `semaphore` builtins** (or solid class equivalents with blocking put/get)
6. **Constraints + `randomize()` / `randomize() with`** — start unconstrained `rand`, then solver
7. **`$cast` / `$typename` hardening** for factory patterns
8. **Covergroups** — functional coverage
9. **DPI-C** — optional but common in real flows

## Tier B — Library / methodology (on top of Tier A)

10. Grow [`uvm/`](../uvm/) toward Accellera-shaped APIs: reporting, phases/objections, factory, `config_db`, TLM, sequences
11. Smoke: trimmed “hello UVM”, then larger Accellera UVM 1.2 slices as features land

## Already usable baseline (do not re-do first)

Classes, packages, strings, queues/dynamic arrays, locators/reductions/ordering, chained calls — extend only when a UVM example needs a gap.

## Out of scope

- Claiming “UVM 1.2 supported” until Accellera’s library compiles and runs
- Opening PRs to `steveicarus/iverilog` for this track (see [WORKFLOW.md](WORKFLOW.md))
