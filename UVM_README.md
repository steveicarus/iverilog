# Icarus Verilog — UVM Experimental Track

> **Experimental / research project.** Not for production or tapeout.
> This is **not** an official Accellera UVM port.
> **Do not open pull requests to [steveicarus/iverilog](https://github.com/steveicarus/iverilog) for this UVM track.**

This repository is `muhammadjawadkhan/iverilog-uvm`: a full Icarus Verilog tree plus a UVM-oriented layer under [`uvm/`](uvm/) and a feature roadmap under [`docs/`](docs/).

| Remote | Role |
|--------|------|
| `origin` | `muhammadjawadkhan/iverilog-uvm` (push here) |
| `upstream` | `steveicarus/iverilog` (**fetch/merge only** — never PR UVM work there) |

## Quick links

- [docs/ROADMAP.md](docs/ROADMAP.md) — SV/UVM features to add one by one
- [docs/STATUS.md](docs/STATUS.md) — what works today
- [docs/WORKFLOW.md](docs/WORKFLOW.md) — branch / PR rules for this fork
- [examples/hello_uvm](examples/hello_uvm) — smoke example using `uvm/`
- Upstream Icarus build docs: see [README.md](README.md)

## Goal

Grow SystemVerilog compiler support and a UVM-like / Accellera-shaped library **in this fork only**, feature by feature, until larger slices of UVM 1.2 become viable.
