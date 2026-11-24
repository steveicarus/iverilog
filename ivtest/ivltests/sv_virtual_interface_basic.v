// Check that virtual interface variables can be declared, assigned, copied and
// compared, and that interface members can be read and written through them
// from a module, a class and a task argument.

interface bus_if;
  logic clk = 0;
  logic [7:0] data = 0;
  real r = 0.0;
endinterface

class drv;
  virtual bus_if vif;
  int cnt = 0;
  function new(virtual bus_if v);
    vif = v;
  endfunction
  task put(input [7:0] d);
    vif.data = d;
  endtask
  task putnb(input [7:0] d);
    vif.data <= d;
  endtask
  function [7:0] get();
    return vif.data;
  endfunction
endclass

module top;
  bus_if b0();
  bus_if b1();
  virtual bus_if v;
  virtual interface bus_if w;
  drv d;
  int fails = 0;

  task automatic chk(input virtual bus_if h, input [7:0] exp);
    if (h.data !== exp) begin
      $display("FAILED: got %h, expected %h", h.data, exp);
      fails = fails + 1;
    end
  endtask

  initial begin
    if (v != null) begin $display("FAILED: not null"); fails++; end
    v = b0;
    w = b1;
    if (v == null || w == null) begin $display("FAILED: null"); fails++; end
    if (v == w) begin $display("FAILED: same"); fails++; end
    v.data = 8'h11;
    w.data = 8'h22;
    if (b0.data !== 8'h11 || b1.data !== 8'h22) begin $display("FAILED: write %h %h", b0.data, b1.data); fails++; end
    b0.data = 8'h33;
    if (v.data !== 8'h33 || w.data !== 8'h22) begin $display("FAILED: read"); fails++; end
    w = v;
    if (w != v || w.data !== 8'h33) begin $display("FAILED: copy"); fails++; end
    w.data[3:0] = 4'hf;
    if (b0.data !== 8'h3f) begin $display("FAILED: part %h", b0.data); fails++; end
    v.data <= 8'h44;
    if (b0.data !== 8'h3f) begin $display("FAILED: nb early"); fails++; end
    #1;
    if (b0.data !== 8'h44) begin $display("FAILED: nb"); fails++; end
    v.r = 2.5;
    if (b0.r != 2.5 || v.r != 2.5) begin $display("FAILED: real"); fails++; end
    d = new(b1);
    d.put(8'h55);
    chk(b1, 8'h55);
    chk(d.vif, 8'h55);
    if (d.get() !== 8'h55 || d.vif.data !== 8'h55) begin $display("FAILED: class"); fails++; end
    d.vif = b0;
    d.putnb(8'h66);
    #1;
    chk(v, 8'h66);
    chk(b1, 8'h55);
    d.vif.data += 1;
    chk(b0, 8'h67);
    v = null;
    if (v != null) begin $display("FAILED: null assign"); fails++; end
    if (fails == 0) $display("PASSED");
  end
endmodule
