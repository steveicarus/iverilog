# Status

Last updated: 2026-07-21

## Accellera UVM 1.2

**Not supported.** Official Accellera UVM will not compile on stock Icarus or on this fork yet.

## This fork

| Area | Status |
|------|--------|
| Icarus Verilog tree | Fork of `steveicarus/iverilog` (`master` @ merge of SV array ordering work) |
| [`uvm/`](../uvm/) | Seeded from IVL_UVM (VerifWorks) — messaging, CLP, stub phases, poor-man’s mailbox/semaphore; **not** Accellera-compatible |
| [`examples/hello_uvm`](../examples/hello_uvm) | Smoke TB for the seeded library (`Makefile` included) |
| Parameterized classes | **Partial** on `feat/param-classes` (merged): ANSI `class C #(type T = int, parameter int W = 8);` parses into the class scope and elaborates with **defaults**. Explicit specializations `C#(byte)` / overrides not done yet. See [`examples/param_classes`](../examples/param_classes). |
| Associative arrays | **Partial** on `feat/assoc-array`: string-keyed `int aa[string]` / `int aa[*]` with size/num/exists/delete/foreach/copy. See [`docs/assoc-array.md`](assoc-array.md) and [`examples/assoc_array`](../examples/assoc_array). |
| Virtual interfaces | **Partial** on `feat/virtual-interface`: `virtual interface T` as class property / TF arg; assign interface instance; member R/W; `@(posedge vif.clk)`. See [`docs/virtual-interface.md`](virtual-interface.md) and [`examples/virtual_interface`](../examples/virtual_interface). |
| Clocking blocks | **Partial** on `feat/clocking-blocks`: interface-local `clocking`; `@(bif.cb)`; `cb.sig` R/W with `#0` skew. See [`docs/clocking.md`](clocking.md) and [`examples/clocking`](../examples/clocking). |
| Constraints / randomize | Missing |
| Covergroups / DPI | Missing |

## Remotes

- `origin` → https://github.com/muhammadjawadkhan/iverilog-uvm
- `upstream` → https://github.com/steveicarus/iverilog (fetch only for this track)
