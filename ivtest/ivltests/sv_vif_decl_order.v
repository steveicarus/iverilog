// interface declared after its use, function returning a handle, package
// variable, old-style task local, real member
package pkg;
  virtual bus_if pv;
endpackage

class agent;
  virtual bus_if vif;
  function virtual bus_if get();
    return vif;
  endfunction
endclass

module test;
  import pkg::*;
  bus_if b1();
  bus_if b2();
  virtual bus_if v;
  agent a;
  bit pass = 1;

  task poke;
    input [15:0] x;
    virtual bus_if lv;
    begin
      lv = b2;
      lv.data = x;
    end
  endtask

  initial begin
    b1.data = 16'h00ff;
    b2.data = 16'h1234;
    b1.r = 1.5;
    b2.r = 2.5;
    v = b2;
    if (v.data !== 16'h1234) begin $display("FAILED: read b2 %h", v.data); pass = 0; end
    v = b1;
    if (v.data !== 16'h00ff) begin $display("FAILED: read b1 %h", v.data); pass = 0; end
    if (v.r != 1.5) begin $display("FAILED: real b1 %f", v.r); pass = 0; end
    v = b2;
    if (v.r != 2.5) begin $display("FAILED: real b2 %f", v.r); pass = 0; end
    v.r = 3.5;
    if (b2.r != 3.5 || b1.r != 1.5) begin $display("FAILED: real write"); pass = 0; end
    a = new;
    a.vif = b1;
    v = a.get();
    if (v != b1) begin $display("FAILED: function result"); pass = 0; end
    if (a.get() == b2) begin $display("FAILED: function compare"); pass = 0; end
    pv = b2;
    v = pkg::pv;
    if (v.data !== 16'h1234) begin $display("FAILED: package var"); pass = 0; end
    poke(16'hbeef);
    if (b2.data !== 16'hbeef || b1.data !== 16'h00ff) begin $display("FAILED: task local"); pass = 0; end
    if (pass) $display("PASSED");
  end
endmodule

interface bus_if #(parameter W = 16);
  logic [W-1:0] data;
  real r;
endinterface
