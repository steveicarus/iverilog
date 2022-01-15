
module DFF
  (output reg Q,
   input wire D,
   input wire CLK,
   input wire RST
   /* */);

   always @(posedge CLK or posedge RST)
     if (RST)
       Q <= 0;
     else
       Q <= D;

endmodule // dut

module main;

   wire q;
   reg	d, clk, rst;
   DFF dut (.Q(q), .D(d), .CLK(clk), .RST(rst));

   initial begin
      clk <= 1;
      d <= 1;

      #1 rst <= 1;
      #1 if (q !== 1'b0) begin
	 $display("FAILED -- RST=%b, Q=%b", rst, q);
	 $finish;
      end

      #1 rst <= 0;
      #1 if (q !== 1'b0) begin
	 $display("FAILED -- RST=%b, Q=%b", rst, q);
	 $finish;
      end

      #1 clk <= 0;
      #1 clk <= 1;
      #1 if (q !== d) begin
	 $display("FAILED -- Q=%b, D=%b", q, d);
	 $finish;
      end

      $display("PASSED");
      $finish;
   end

endmodule // main
