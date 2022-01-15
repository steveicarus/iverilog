module test ();

   // Note the implicit declaration of w.
   assign  w = 1'b1;

   initial begin
      #1 if (w !== 1'b1) begin
	 $display("FAILED");
	 $finish;
      end

      $display("PASSED");
   end

endmodule
