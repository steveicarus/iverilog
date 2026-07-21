# Unconstrained `randomize()` / `rand` (Tier A #6, first slice)

Status: **partial** — unconstrained integral `rand`/`randc` only; no constraint solver.

## Supported in this slice

```systemverilog
class pkt;
  rand bit [7:0] a;
  rand bit [3:0] b;
  bit [7:0] c;              // non-rand left alone
  function new(); c = 8'h11; endfunction
endclass

pkt p = new;
bit ok = p.randomize();     // returns 1; assigns random values to a,b
```

- `obj.randomize()` returns `1` on success (null / unknown class type → `0`)
- Assigns random 0/1 bits to all `rand` and `randc` **integral** (`bit`/`logic`/`reg` packed) instance properties
- Non-rand properties are unchanged
- `randc` is treated like `rand` for now (no cyclic guarantee)

## Encoding

| Layer | Role |
|-------|------|
| Parse | `rand`/`randc` already property qualifiers; `constraint` / `randomize() with` remain **sorry** |
| Elab | `obj.randomize()` → `$ivl_randomize(obj)` (expr or task) |
| Target API | `ivl_type_prop_rand()` exposes qualifiers to codegen |
| Codegen | `%urandom <wid>` + `%store/prop/v` per rand property |
| Runtime | `%urandom` fills vec4 with RNG bits (`std::mt19937`) |

## Deferred (do not claim)

- Constraint blocks / solver
- `randomize() with { ... }`
- `rand_mode` / `constraint_mode`
- True `randc` cyclic behavior
- Randomizing non-integral properties (reals, strings, class handles, arrays)
- `std::randomize` procedural form / inline constraints

## Example

[`examples/randomize/randomize_basic.sv`](../examples/randomize/randomize_basic.sv) — prints `PASSED`.

```bash
./install/bin/iverilog -g2012 -o /tmp/rand.vvp examples/randomize/randomize_basic.sv
./install/bin/vvp /tmp/rand.vvp
```

See also [STATUS.md](STATUS.md) and [ROADMAP.md](ROADMAP.md).
