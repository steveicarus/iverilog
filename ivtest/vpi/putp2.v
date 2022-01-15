module main;

   integer x;

   initial begin
      x = $copy_test(1);
      if (x !== 1) begin
	 $display("FAILED -- x == %b (should be 1)", x);
	 $finish;
      end

      x = $copy_test(8);
      if (x !== 8) begin
	 $display("FAILED -- x == %b (should be 8)", x);
	 $finish;
      end

      $display("PASSED");
   end

endmodule // main
