module test (clk, in, out);
   input clk;
   input [15:0] in;
   output [4:0] out;

   reg [4:0] out;
   (* ivl_synthesis_on *)
   always @(posedge clk) begin
      // In PR#685, this caused an assertion with iverilog -S
      out = (in >= 16) ? 16 : in;
   end

endmodule

module main;

   reg clk;
   reg [15:0] value;
   wire [4:0] sat;

   test dut (clk, value, sat);

   (* ivl_synthesis_off *)
   initial begin
      value = 0;
      clk = 1;
      for (value = 0 ;  value < 'h15 ;  value = value+1) begin
	 #1 clk = 0;
	 #1 clk = 1;
	 #1 if ((value > 16) && (sat !== 5'd16)) begin
	    $display("FAILED -- value=%d, sat=%b", value, sat);
	    $finish;
	 end

	 if ((value <= 16) && (value !== sat)) begin
	    $display("FAILED -- value=%d, sat=%b", value, sat);
	    $finish;
	 end
      end // for (value = 0 ;  value < 'h15 ;  value = value+1)

      $display("PASSED");
   end // initial begin

endmodule // main
