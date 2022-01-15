module main;

   function [15:0] sum;
      input [15:0] a;
      input [15:0] b;

      sum = a + b;
   endfunction // sum

   reg		   clk;
   reg [15:0]	   d, e, out;
   (* ivl_synthesis_on *)
   always @(posedge clk)
     out <= sum(d, e);


   initial begin
      clk = 0;
      d = 0;
      e = 0;

      #1 clk = 1;
      #1 clk = 0;

      if (out !== 16'd0) begin
	 $display("FAILED -- sum(%0d,%d) --> %0d", d, e, out);
	 $finish;
      end

      d = 5;
      e = 13;

      #1 clk = 1;
      #1 clk = 0;

      if (out !== 16'd18) begin
	 $display("FAILED -- sum(%0d,%d) --> %0d", d, e, out);
	 $finish;
      end

      $display("PASSED");
      $finish;
   end
endmodule // main
