
/*
 * This is a post-synthesis test for the blif_sign_ext.v test. Run this
 * simulation in these steps:
 *
 *   $ iverilog -tblif -o foo.blif blif_sign_ext.v
 *   $ abc
 *   abc 01> read_blif foo.blif
 *   abc 02> write_verilog foo.v
 *   abc 03> quit
 *   $ iverilog -g2009 -o foo.vvp blif_sign_ext_tb.v foo.v
 *   $ vvp foo.vvp
 */
module main;

   parameter W=3, WO=5;
   reg signed [W:0]  D;
   reg signed [WO:0] q;
   wire [WO:0] Q;

   sign_ext se(.\D[3] (D[3]), .\D[2] (D[2]), .\D[1] (D[1]), .\D[0] (D[0]),
           .\Q[5] (Q[5]), .\Q[4] (Q[4]), .\Q[3] (Q[3]), .\Q[2] (Q[2]), .\Q[1] (Q[1]), .\Q[0] (Q[0]));

   int		 ddx;
   initial begin
      for (ddx = 0 ; ddx < 1 << (W+1) ; ddx = ddx+1) begin
	 D = ddx[W:0];
         q = D;

         $display("D = %b, q = %b", D, q);

         #1;
	 if (Q !== q) begin
	    $display("FAILED -- D=%b, Q=%b (should be %b)", D, Q, q);
	    $finish;
         end
      end
      $display("PASSED");
   end

endmodule // main
