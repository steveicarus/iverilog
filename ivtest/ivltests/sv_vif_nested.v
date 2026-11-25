// nested class chain, handle passed between objects, instance in a submodule
interface bus_if;
  logic [7:0] data;
endinterface

class drv;
  virtual bus_if vif;
  function new(virtual bus_if v); vif = v; endfunction
endclass

class env;
  drv d;
  virtual bus_if vif;
  function new(virtual bus_if v);
    vif = v;
    d = new(vif);
  endfunction
endclass

module sub;
  bus_if bs();
endmodule

module test;
  bus_if b1();
  bus_if b2();
  sub s();
  virtual bus_if v;
  env e;
  drv dd;
  bit pass = 1;

  initial begin
    b1.data = 8'h01; b2.data = 8'h02; s.bs.data = 8'h03;
    v = s.bs;
    if (v.data !== 8'h03) begin $display("FAILED: sub instance %h", v.data); pass = 0; end
    e = new(b2);
    dd = e.d;
    if (dd.vif != b2) begin $display("FAILED: nested handle"); pass = 0; end
    dd.vif.data = 8'h22;
    if (b2.data !== 8'h22 || b1.data !== 8'h01) begin $display("FAILED: nested write %h %h", b1.data, b2.data); pass = 0; end
    if (dd.vif.data !== 8'h22) begin $display("FAILED: nested read"); pass = 0; end
    dd.vif = e.vif;
    e.vif = b1;
    if (dd.vif.data !== 8'h22 || e.vif.data !== 8'h01) begin $display("FAILED: nested copy"); pass = 0; end
    if (pass) $display("PASSED");
  end
endmodule
