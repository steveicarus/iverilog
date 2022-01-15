module main;

   integer cnt;
   reg [31:0] a1, a2, a3;

   initial begin
      cnt = 'bx;
      cnt = $sscanf("123 hex beaf bin 01xz", "%d hex %h bin %b", a1, a2, a3);
      if (cnt != 3) begin
	 $display("FAILED -- cnt = %d", cnt);
	 $finish;
      end

      if (a1 !== 123) begin
	 $display("FAILED -- a1 = %d (b%b)", a1, a1);
	 $finish;
      end

      if (a2 !== 'hbeaf) begin
	 $display("FAILED -- a2 = %h", a2);
	 $finish;
      end

      if (a3 !== 'b01xz) begin
	 $display("FAILED -- a3 = %h", a3);
	 $finish;
      end

      $display("PASSED");
   end

endmodule
