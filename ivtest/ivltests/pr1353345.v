module main;

   parameter use_wid = 4;

   reg  [use_wid-1:0] d;
   wire [use_wid-1:0] q;
   reg		      clk;

   B #(.wid(use_wid)) dut (.Q(q), .D(d), .C(clk));

   initial begin
      clk = 0;
      d = 4'b0000;

      #1 clk = 1;
      #1 clk = 0;

      if (q !== 4'b0000) begin
	 $display("FAILED -- d=%b, q=%b", d, q);
	 $finish;
      end

      d = 4'b1111;
      #1 clk = 1;
      #1 clk = 0;

      if (q !== 4'b1111) begin
	 $display("FAILED -- d=%b, q=%b", d, q);
	 $finish;
      end

      $display("PASSED");
   end

endmodule // main

/*
 * although the wid paramter is default to 3 in this module, the point
 * of this test is to have the instantiating module (main) give a
 * different value and have that value properly handlued in all the
 * situations of this module.
 */
module B
  #(parameter wid = 3)
  (output [wid-1:0] Q,
   input  [wid-1:0] D,
   input C);

   // the override from main will cause this to be a width of 4.
   prim U [wid-1:0] (Q, D, C);
   //prim U [wid-1:0] (.Q(Q), .D(D), .C(C));

endmodule // B

module prim(output reg Q, input D, C);

   always @(posedge C)
     Q <= D;

endmodule // prim
