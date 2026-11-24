// always_ff processes that use a virtual interface
interface reg_if;
  logic       clk = 0;
  logic [7:0] d;
  logic [7:0] q;
endinterface

module dut(reg_if bus);
  virtual reg_if v = bus;

  always_ff @(posedge v.clk) v.q <= v.d;
endmodule

module test;
  reg_if r1();
  reg_if r2();
  virtual reg_if v = r2;
  bit pass = 1;

  dut u1(r1);

  initial begin
    r1.d = 8'h11; r2.d = 8'h22;
  end

  always_ff @(negedge v.clk) v.q <= v.d + 8'h01;

  initial begin
    #2 r1.clk = 1; r2.clk = 1;
    #1 if (r1.q !== 8'h11) begin $display("FAILED: r1.q = %h", r1.q); pass = 0; end
    if (r2.q !== 8'hxx) begin $display("FAILED: r2.q = %h", r2.q); pass = 0; end
    r1.d = 8'h33;
    #2 r1.clk = 0; r2.clk = 0;
    #1 if (r1.q !== 8'h11) begin $display("FAILED: r1.q = %h", r1.q); pass = 0; end
    if (r2.q !== 8'h23) begin $display("FAILED: r2.q = %h", r2.q); pass = 0; end
    #2 r1.clk = 1;
    #1 if (r1.q !== 8'h33) begin $display("FAILED: r1.q = %h", r1.q); pass = 0; end
    if (pass) $display("PASSED");
    $finish;
  end
endmodule
