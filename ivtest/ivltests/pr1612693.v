/* pr1612693.v */

module test ();
   reg [9:0] col;
   wire [9:0] xsize;
   // The setup for this expression caused an assertion at run time
   // according to pr1612693.
   wire       vschg = (col == (xsize>>1));

   initial begin
      #1 $display("PASSED");
   end

endmodule
