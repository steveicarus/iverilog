/*
 * This is a simplified version of the test program for issue 1323691
 * from the iverilog bugs database.
 */
`timescale 1ns/1ns


module main;

   parameter early_empty=1;

   reg	     re;

   reg	     rc_isEmpty, rc_willBeEmpty;
   wire      empty;

   assign    empty = (early_empty!=0) ?
			        rc_willBeEmpty && re || rc_isEmpty :
			        rc_isEmpty;


   initial begin
      rc_isEmpty <= 1'bx;
      rc_willBeEmpty <= 1'b1;
      re <= 1'b0;
      rc_isEmpty <= 1'b0;

      #1 if (empty !== 1'b0) begin
	 $display("FAILED -- empty == %b (s.b. 0)", empty);
	 $finish;
      end

      rc_isEmpty <= 1;
      #1 if (empty !== 1'b1) begin
	 $display("FAILED -- empty == %b (s.b. 1)", empty);
	 $finish;
      end

      $display("PASSED");
   end
endmodule
