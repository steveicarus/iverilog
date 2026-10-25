`timescale 1ns/10ps

module top;
  reg in, clk;
  wire outp, outn;

  dut idut(outp, outn, in , clk);

  initial begin
    clk = 1'b0;
    forever #1 clk = ~clk;
  end

  initial begin
    $monitor($realtime,,outp,outn,,in,clk);
    #2;
    in = 1'b0;
    #2;
    in = 1'b1;
    #2;
    #0.75;
    $display("PASSED");
    $finish(0);
  end
endmodule

module dut(output reg qp, output reg qn, input reg a, input reg clk);

  always @(posedge clk) qp <= a;
  always @(negedge clk) qn <= a;

  specify
    (posedge clk *> qp) = 0.25;
    (negedge clk *> qn) = 0.25;
  endspecify
endmodule
