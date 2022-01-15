/***************************************************************
** Author: Oswaldo Cadenas (oswaldo.cadenas@gmail.com)
** Date: September 26 2011
**
** Test: Intended to test parametric counter in timebase.vhd
**       the counter has parameters: N for counter length and
         VALUE to flag when the count reaches this value
**
** Four counter instances are created here:
**      duv1 with counter default parameters for N and VALUE
**      duv2 with N1, V1 for parameter N, VALUE respectively
**      duv3 with N2, V2 for parameters N, VALUE respectively
**      duv4 with N2 replacing N and VALUE left as default
**
** The test for a long time making sure each of the four counter flags TICK become one
**************************************************************************************/

module test;
  parameter integer T = 25;
  parameter integer N1 = 8;
  parameter integer N2 = 17;
  parameter integer V1 = 200;
  parameter integer V2 = 17'h16C8A;

  bit clk = 0, reset = 0;
  wire [11:0]   count1;
  wire [N1-1:0] count2;
  wire [N2-1:0] count3;
  wire [N2-1:0] count4;

  wire tick1, tick2, tick3, tick4;
  reg tick1_reg, tick2_reg, tick3_reg, tick4_reg;

  initial forever #(T) clk = !clk;


  initial begin
    @(negedge clk);
    reset = 1'b1;
    repeat(6) @(negedge clk);
    reset = 1'b0;
  end

  // duv1 switch
  always @(posedge clk, posedge reset)
	if (reset) tick1_reg <= 1'b0;
	else if (tick1) tick1_reg <= 1'b1;

  // duv2 switch
  always @(posedge clk, posedge reset)
	if (reset) tick2_reg <= 1'b0;
	else if (tick2) tick2_reg <= 1'b1;

  // duv3 switch
  always @(posedge clk, posedge reset)
	if (reset) tick3_reg <= 1'b0;
	else if (tick3) tick3_reg <= 1'b1;

  // duv4 switch
  always @(posedge clk, posedge reset)
	if (reset) tick4_reg <= 1'b0;
	else if (tick4) tick4_reg <= 1'b1;

  initial begin
    #(V2*2*T + 1000);
    if (tick1_reg != 1 || tick2_reg != 1 || tick3_reg != 1 || tick4_reg != 1) begin
      $display ("Counting FAILED");
      $finish;
    end
    else begin
      $display ("PASSED");
      #20;
      $finish;
  end
end

  timebase duv1 (.clock(clk), .reset(reset), .enable(1'b1), .tick(tick1), .count_value(count1) ); // default parameters
  timebase #(.n(N1), .value(V1))
           duv2 (.clock(clk), .reset(reset), .enable(1'b1), .tick(tick2), .count_value(count2) ); // N1, V1 parameters
  timebase #(N2, V2)
           duv3 (.clock(clk), .reset(reset), .enable(1'b1), .tick(tick3), .count_value(count3) ); // N2, V2 parameters
  timebase #(.n(N2))
           duv4 (.clock(clk), .reset(reset), .enable(1'b1), .tick(tick4), .count_value(count4) ); // only one parameter modified

endmodule
