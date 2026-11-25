// initialiser, always block, handle in a block, two interface types
interface bus_if;
  logic [7:0] data;
  logic       clk;
endinterface

interface other_if;
  logic [7:0] data;
endinterface

module test;
  bus_if b1();
  bus_if b2();
  other_if o1();
  virtual bus_if v = b2;
  virtual other_if ov;
  integer cnt = 0;
  bit pass = 1;

  always @(posedge v.clk) cnt = cnt + 1;

  initial begin
    b1.clk = 0; b2.clk = 0;
    if (v != b2) begin $display("FAILED: initialiser"); pass = 0; end
    #1 b2.clk = 1;
    #1 b2.clk = 0; b1.clk = 1;
    #1 b1.clk = 0;
    if (cnt !== 1) begin $display("FAILED: always count %0d", cnt); pass = 0; end
    ov = o1;
    ov.data = 8'h5A;
    if (o1.data !== 8'h5A) begin $display("FAILED: other_if"); pass = 0; end
    begin
      virtual bus_if lv;
      lv = b1;
      lv.data = 8'h99;
      if (b1.data !== 8'h99) begin $display("FAILED: block local"); pass = 0; end
    end
    if (pass) $display("PASSED");
  end
endmodule
