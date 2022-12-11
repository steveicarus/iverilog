
module main;
   initial begin
      int idx;
      idx = 1;
      for ( ; idx < 5 ; idx += 1) begin
	 $display("... %02d", idx);
      end
      if (idx !== 5) begin
	 $display("FAILED -- idx=%0d", idx);
	 $finish;
      end
      $display("PASSED");
      $finish;
   end
endmodule // main
