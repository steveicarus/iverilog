/*
 * Exhaustive check of all the subtract results.
 */
module main;

   wire [7:0] out;
   reg [7:0] A, B;

   sub8 dut(.out(out), .A(A), .B(B));

   reg	     error = 0;
   integer   adx, bdx;

   initial begin
      A = 0;
      B = 0;

      for (adx = 0 ;  adx < 256 ;  adx = adx + 1) begin
	 A = adx;
	 for (bdx = 0 ;  bdx < 256 ;  bdx = bdx + 1) begin
	    B = bdx;
	    #1 $write("%b - %b: %b", A, B, out);
	    if (out !== (A - B)) begin
	       $display(" ERROR");
	       error = 1;
	    end else begin
	       $display(" OK");
	    end

	 end // for (bdx = 0 ;  bdx < 256 ;  bdx += 1)
      end // for (adx = 0 ;  adx < 256 ;  adx = adx + 1)

      if (error == 0)
	$display("PASSED");
      else
	$display("FAILED");

   end // initial begin
endmodule // main
