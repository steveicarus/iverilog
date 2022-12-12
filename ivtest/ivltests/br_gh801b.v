
module main;
   initial begin
      int idx;
      for (idx = 1 ; idx < 5 ; ) begin
	 $display("... %02d", idx);
	 idx += 1;
      end
      if (idx !== 5) begin
	 $display("FAILED -- idx=%0d", idx);
	 $finish;
      end
      idx = 2;
      for ( ; idx < 5 ; ) begin
	 $display("... %02d", idx);
	 idx += 1;
      end
      $display("PASSED");
      $finish;
   end
endmodule // main
