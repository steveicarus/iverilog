module main;

   real rfoo;
   reg [5:0] x, y;

   initial begin
      rfoo = 1.0;
      x = 5;
      y = 2;

      rfoo = rfoo + x%y;
      x = rfoo;
      $display("rfoo = %f, x=%d", rfoo, x);
      if (x !== 5'd2) begin
	 $display("FAILED");
	 $finish;
      end

      if (rfoo != 2.0) begin
	 $display("FAILED");
	 $finish;
      end

      $display("PASSED");
   end

endmodule // main
