/***************************************************************
** Author: Oswaldo Cadenas (oswaldo.cadenas@gmail.com)
** Date: September 26 2011
**
** Test: Intended to test a system composed of some parametric system
**       that has an adder, a register and an incrementer
**       Each module has parameter: N for data length
**
** A system is given parameter P and should return P-1, this is run for M test vectors
**************************************************************************************/

module test;
parameter integer T = 25; // for the clock period
parameter integer P = 1000; // a constant passed to the system under test
parameter integer M = 200;  // number of test vectors

int i;

bit clk = 0, reset = 0;

byte unsigned x;
wire [10:0] y;

initial forever #(T) clk = !clk;

initial begin
    @(negedge clk);
    reset = 1'b1;
    repeat(6) @(negedge clk);
    reset = 1'b0;
end

initial begin
  @(posedge reset);
  @(negedge reset);
  for (i = 0; i < M; i=i+1) begin
    x = {$random} % 255;
    @(negedge clk);
    if (y !== P-1) begin
      $display ("ERROR");
      $finish;
    end
  end
  #100;
  $display ("PASSED");
  $finish;
end

const_system #(P) duv (.clk(clk), .reset(reset), .x(x), .y(y) );



endmodule
