// Check blocking reads and writes of interface members through virtual
// interfaces that are held in classes, task arguments, arrays and
// interface ports.
interface bus_if;
  logic [7:0] data = 0;
  logic [3:0] nib = 0;
endinterface

module sub(bus_if p);
endmodule

class holder;
  local virtual bus_if vif;
  function new(virtual bus_if v);
    vif = v;
  endfunction
  function logic [7:0] get();
    return vif.data;
  endfunction
  task put(input logic [7:0] d);
    vif.data = d;
    this.vif.nib[2:1] = d[1:0];
  endtask
endclass

module test;
  bus_if b[2:0]();
  bus_if p0();
  sub s0(p0);
  virtual bus_if va[2];
  virtual bus_if v;
  holder h;
  bit failed = 0;

  task automatic poke(virtual bus_if t, input logic [7:0] d);
    t.data = d;
  endtask

  function automatic virtual bus_if pick(int i);
    return va[i];
  endfunction

  initial begin
    va[0] = b[1];
    va[1] = p0;
    poke(va[0], 8'h12);
    poke(va[1], 8'h34);
    if (b[1].data !== 8'h12 || p0.data !== 8'h34) failed = 1;
    v = pick(1);
    if (v != p0 || v.data !== 8'h34) failed = 1;
    h = new(b[2]);
    h.put(8'h03);
    if (b[2].data !== 8'h03 || b[2].nib !== 4'b0110) failed = 1;
    if (h.get() !== 8'h03) failed = 1;
    v = va[0];
    v.data[7:4] = 4'ha;
    if (b[1].data !== 8'ha2) failed = 1;
    if (failed)
      $display("FAILED");
    else
      $display("PASSED");
  end
endmodule
