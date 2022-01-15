/*
 * This example is a distillation of the essence of PR#993.
 * Or at least the essence that led to a bug report.
 */

module main;

   integer length;
   wire [31:0] length_bits  = ((length * 8 )/11)+(((length * 8 )%11) != 0);

   reg [31:0] length_bits2;

   initial begin
      for (length = 1 ;  length < 56 ;  length = length + 1) begin
	 length_bits2 = ((length * 8 )/11)+(((length * 8 )%11) != 0);

	 #1 $display("length=%3d, length_bits=%3d (%3d)",
		     length, length_bits, length_bits2);

	 if (length_bits != length_bits2) begin
	    $display("FAILED - Expressions have different results.");
	    $finish;
	 end
      end // for (length = 1 ;  length < 56 ;  length = length + 1)

      $finish(0);
   end

endmodule // main
