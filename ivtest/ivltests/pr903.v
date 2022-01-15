/*
 * This tests some compile-time division of very long constants.
 */
module main;

   initial begin
      $display('h9000_0000_0000_0000_0000 / 'h3000_0000_0000_0000_0000);
      if ('h9000_0000_0000_0000_0000 / 'h3000_0000_0000_0000_0000 !== 3) begin
	 $display("FAILED");
	 $finish;
      end

      $display('h5000_0000_0000_0000_0000 / 'h5000_0000_0000_0000_0000);
      if ('ha000_0000_0000_0000_0000 / 'h5000_0000_0000_0000_0000 !== 2) begin
	 $display("FAILED");
	 $finish;
      end

      $display("PASSED");
   end

endmodule // main
