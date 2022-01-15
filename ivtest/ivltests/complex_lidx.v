module main;

   parameter MAP = 16'h0123;
   parameter VAL = 32'h44_33_22_11;
   wire [31:0] value;

   generate
      genvar m, n;
      for (m = 0 ; m < 4 ; m = m+1) begin : drv
	 for (n = 0 ; n < 8 ;  n = n+1) begin : drv_n
	    assign value[8*MAP[4*m +: 4] + n] = VAL[8*m+n +: 1];
	 end
      end
   endgenerate

   initial begin
      #1 $display("value = %h", value);
      if (value !== 32'h11_22_33_44) begin
	 $display("FAILED");
	 $finish;
      end
      $display("PASSED");
   end

endmodule // main
