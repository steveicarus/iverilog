
module main;

   parameter WID = 4;
   parameter SWID = 2;

   reg [WID-1:0]  D;
   reg [SWID-1:0] S;
   wire		  Q;

   muxN dut(.\D[3] (D[3]), .\D[2] (D[2]), .\D[1] (D[1]), .\D[0] (D[0]),
	    .\S[1] (S[1]), .\S[0] (S[0]),
	    .Q(Q));

   integer	  idx, sdx;
   initial begin
      for (idx = 0 ; idx < 50 ; idx += 1) begin
	 D = $random;

	 for (sdx = 0 ; sdx < (1<<SWID) ; sdx = sdx+1) begin
	    S = sdx[SWID-1:0];
	    #1 ;
	    if (Q !== D[S]) begin
	       $display("FAILED = D=%b, S=%0d, Q=%b", D, S, Q);
	       $finish;
	    end
	 end
      end // for (idx = 0 ; idx < 50 ; idx += 1)
      $display("PASSED");
   end // initial begin

endmodule // main
