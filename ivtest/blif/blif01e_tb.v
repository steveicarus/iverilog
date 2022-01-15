
/*
 * This is a post-synthesis test for the blif01a.v test. Run this
 * simulation in these steps:
 *
 *   $ iverilog -tblif -o foo.blif blif01a.v
 *   $ abc
 *   abc 01> read_blif foo.blif
 *   abc 02> write_verilog foo.v
 *   abc 03> quit
 *   $ iverilog -g2009 -o foo.vvp blif02a_tb.v foo.v
 *   $ vvp foo.vvp
 */
module main;

   parameter WID = 4;
   reg [WID-1:0] A, B;
   wire		 QE, QN, QGT, QGE;

   cmpN ucmp(.\A[3] (A[3]), .\A[2] (A[2]), .\A[1] (A[1]), .\A[0] (A[0]),
	     .\B[3] (B[3]), .\B[2] (B[2]), .\B[1] (B[1]), .\B[0] (B[0]),
	     .QE(QE), .QN(QN), .QGT(QGT), .QGE(QGE));

   int		 adx;
   int		 bdx;
   initial begin
      for (bdx = 0 ; bdx[WID]==0 ; bdx = bdx+1) begin
	 for (adx = 0 ; adx[WID]==0 ; adx = adx+1) begin
	    A <= adx[WID-1:0];
	    B <= bdx[WID-1:0];
	    #1 ;
	    if (QE !== (adx[WID-1:0]==bdx[WID-1:0])) begin
	       $display("FAILED -- A=%b, B=%b, QE=%b", A, B, QE);
	       $finish;
	    end
	    if (QN !== (adx[WID-1:0]!=bdx[WID-1:0])) begin
	       $display("FAILED -- A=%b, B=%b, QN=%b", A, B, QN);
	       $finish;
	    end
	    if (QGT !== (adx[WID-1:0] > bdx[WID-1:0])) begin
	       $display("FAILED -- A=%b, B=%b, QGT=%b", A, B, QGT);
	       $finish;
	    end
	    if (QGE !== (adx[WID-1:0] >= bdx[WID-1:0])) begin
	       $display("FAILED -- A=%b, B=%b, QGE=%b", A, B, QGE);
	       $finish;
	    end
	 end
      end
      $display("PASSED");
   end

endmodule // main
