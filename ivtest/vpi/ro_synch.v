module main;

   integer val;

   initial begin
      val = 0;
      $delayed_poke(val, 1, -1);
      $delayed_poke(val, 2,  0);
      $delayed_poke(val, 3,  1);
      #1 if (val !== 0) begin
	 $display("FAILED -- val==%0d before legal poke", val);
	 $finish;
      end
      #1 if (val !== 3) begin
	 $display("FAILED -- val==%0d: legal poke didn't happen", val);
	 $finish;
      end

      $display("PASSED");
      $finish(0);
   end

endmodule // main
