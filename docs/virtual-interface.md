# Virtual interfaces (Tier A #3)

Status: **partial** — enough for basic UVM-agent style connectivity.

## Supported in this slice

```systemverilog
interface bus_if;
  logic clk;
  logic [7:0] data;
endinterface

class driver;
  virtual interface bus_if vif;          // class property
  function new(virtual interface bus_if i); // TF argument
    vif = i;                             // assign interface instance
  endfunction
  task drive(logic [7:0] d);
    @(posedge vif.clk);                  // edge wait via VI
    vif.data = d;                        // member write
  endtask
endclass

module top;
  bus_if bif();
  driver drv;
  initial begin
    drv = new(bif);
    // ...
  end
endmodule
```

Also: member reads (`v = vif.data`) and anyedge / negedge waits on VI members.

**Syntax note:** the type must be written as `virtual interface <name>`. Bare `virtual bus_if` is deferred (parser conflicts with `virtual class` / `virtual function`).

## Encoding

| Layer | Role |
|-------|------|
| Parse | `K_virtual_interface` + `virtual_interface_type_t` (class props / TF ports) |
| Netlist | `netvif_t` (`IVL_VT_CLASS` so object load/store reuse works) |
| Elab | `$ivl_vif_new` / `$ivl_vif_get` / `$ivl_vif_wait`; VI member lvals |
| Runtime | `vvp_vif` + `%new/vif`, `%vif/load/vec4`, `%vif/store/vec4`, `%vif/wait` |

A virtual interface is a handle to an interface instance: member access and waits go through the bound signals.

## Example

[`examples/virtual_interface/vif_basic.sv`](../examples/virtual_interface/vif_basic.sv) — prints `PASSED`.

```bash
./install/bin/iverilog -g2012 -o /tmp/vif_basic.vvp examples/virtual_interface/vif_basic.sv
./install/bin/vvp /tmp/vif_basic.vvp
```

## Deferred (do not claim)

- Bare `virtual bus_if` (without the `interface` keyword)
- Virtual interface arrays
- Modport-qualified virtual interfaces (type enforcement)
- Clocking blocks on VI (Tier A #4)
- Parameterized interfaces

See also [STATUS.md](STATUS.md) and [ROADMAP.md](ROADMAP.md).
