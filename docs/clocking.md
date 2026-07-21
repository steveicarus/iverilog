# Clocking blocks (Tier A #4)

Status: **partial** — interface-local clocking enough for basic driver sync.

## Supported in this slice

```systemverilog
interface bus_if(input logic clk);
  logic [7:0] data;
  logic req;
  clocking cb @(posedge clk);
    input  #0 data;   // #1step may be written; treated as #0
    output #0 req;
  endclocking
endinterface

module top;
  logic clk;
  bus_if bif(clk);
  logic [7:0] x;
  initial begin
    bif.data = 8'h3C;
    @(bif.cb);              // wait on clocking event (posedge clk)
    bif.cb.req <= 1'b1;     // NBA to underlying req
    x = bif.cb.data;        // read current value of data
  end
endmodule
```

### Pragmatic semantics (v1)

| Construct | Behavior |
|-----------|----------|
| `@(cb)` / `@(bif.cb)` | Wait on the clocking event (`posedge clk`) |
| `cb.req <= val` | Non-blocking assign to underlying `req` (visible after NBA / next clocking event) |
| `cb.data` (read) | Current value of `data` (sampling skew = `#0`) |
| `#0` / `#1step` skew | Accepted; `#1step` aliases to `#0` if present |

Clocking signal names are **transparent** hierarchical hops: `bif.cb.req` resolves to `bif.req`.

## Example

[`examples/clocking/cb_basic.sv`](../examples/clocking/cb_basic.sv) — prints `PASSED`.

```bash
./install/bin/iverilog -g2012 -o /tmp/cb_basic.vvp examples/clocking/cb_basic.sv
./install/bin/vvp /tmp/cb_basic.vvp
```

## Deferred (do not claim)

- `default clocking`
- `##` cycle delays
- Modport clocking (`modport m (clocking cb);`)
- Complex / non-zero input/output skews
- `inout` clocking drive/sample timing
- `vif.cb` through virtual interfaces (follow-on once VI member lookup covers clocking)

See also [STATUS.md](STATUS.md) and [ROADMAP.md](ROADMAP.md).
