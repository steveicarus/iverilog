# Status

Last updated: 2026-07-21

## Accellera UVM 1.2

**Not supported.** Official Accellera UVM will not compile on stock Icarus or on this fork yet.

## This fork

| Area | Status |
|------|--------|
| Icarus Verilog tree | Fork of `steveicarus/iverilog` (`master` @ merge of SV array ordering work) |
| [`uvm/`](../uvm/) | Seeded from IVL_UVM (VerifWorks) — messaging, CLP, stub phases, poor-man’s mailbox/semaphore; **not** Accellera-compatible |
| [`examples/hello_uvm`](../examples/hello_uvm) | Smoke TB for the seeded library |
| Parameterized classes | **In progress** on `feat/param-classes` |
| Associative arrays | Missing |
| Virtual interfaces | Missing |
| Constraints / randomize | Missing |
| Covergroups / DPI | Missing |

## Remotes

- `origin` → https://github.com/muhammadjawadkhan/iverilog-uvm
- `upstream` → https://github.com/steveicarus/iverilog (fetch only for this track)
