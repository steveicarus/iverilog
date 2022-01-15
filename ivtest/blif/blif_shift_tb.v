
/*
 * This is a post-synthesis test for the blif_shift.v test. Run this
 * simulation in these steps:
 *
 *   $ iverilog -tblif -o foo.blif blif_shift.v
 *   $ abc
 *   abc 01> read_blif foo.blif
 *   abc 02> write_verilog foo.v
 *   abc 03> quit
 *   $ iverilog -g2009 -o foo.vvp blif_shift_tb.v foo.v
 *   $ vvp foo.vvp
 */
module main;

   parameter W=3;
   reg [W:0]  D;
   reg [W:0]  S;

   parameter WO=5;
   wire [WO:0] SHL;
   wire [WO:0] SHR;
   wire [WO:0] ASHL;
   wire [WO:0] ASHR;
   reg [WO:0] shl;
   reg [WO:0] shr;
   reg [WO:0] ashl;
   reg [WO:0] ashr;

`ifdef DUMMY
   shift ss(.D (D), .S (S), .SHL (SHL), .SHR (SHR), .ASHL (ASHL), .ASHR (ASHR));
`else
   shift ss(.\D[3] (D[3]), .\D[2] (D[2]), .\D[1] (D[1]), .\D[0] (D[0]),
           .\S[3] (S[3]), .\S[2] (S[2]), .\S[1] (S[1]), .\S[0] (S[0]),
           .\SHL[5] (SHL[5]), .\SHL[4] (SHL[4]), .\SHL[3] (SHL[3]), .\SHL[2] (SHL[2]), .\SHL[1] (SHL[1]), .\SHL[0] (SHL[0]),
           .\SHR[5] (SHR[5]), .\SHR[4] (SHR[4]), .\SHR[3] (SHR[3]), .\SHR[2] (SHR[2]), .\SHR[1] (SHR[1]), .\SHR[0] (SHR[0]),
           .\ASHL[5] (ASHL[5]), .\ASHL[4] (ASHL[4]), .\ASHL[3] (ASHL[3]), .\ASHL[2] (ASHL[2]), .\ASHL[1] (ASHL[1]), .\ASHL[0] (ASHL[0]),
           .\ASHR[5] (ASHR[5]), .\ASHR[4] (ASHR[4]), .\ASHR[3] (ASHR[3]), .\ASHR[2] (ASHR[2]), .\ASHR[1] (ASHR[1]), .\ASHR[0] (ASHR[0]));
`endif

   int		 ddx;
   int		 sdx;
   initial begin
      for (ddx = 0 ; ddx < 1 << (W+1) ; ddx = ddx+1)
      for (sdx = 0 ; sdx < WO + 2 ; sdx = sdx+1) begin
	 D    = ddx[W:0];
	 S    = sdx[W:0];

         shl  = D << S;
         shr  = D >> S;
         ashl = $signed(D) <<< S;
         ashr = $signed(D) >>> S;

//         $display("D = %b, S = %b", D, S);
//         $display("shl = %b, shr = %b", shl, shr);
//         $display("ashl = %b, ashr = %b", ashl, ashr);

         #1;
	 if (SHL !== shl) begin
	    $display("FAILED -- D=%b, S=%b, SHL=%b (should be %b)", D, S, SHL, shl);
	    $finish;
         end
	 if (SHR !== shr) begin
	    $display("FAILED -- D=%b, S=%b, SHR=%b (should be %b)", D, S, SHR, shr);
	    $finish;
         end
	 if (ASHL !== ashl) begin
	    $display("FAILED -- D=%b, S=%b, ASHL=%b (should be %b)", D, S, ASHL, ashl);
	    $finish;
         end
	 if (ASHR !== ashr) begin
	    $display("FAILED -- D=%b, S=%b, SHL=%b (should be %b)", D, S, ASHR, ashr);
	    $finish;
         end
      end
      $display("PASSED");
   end

endmodule // main
