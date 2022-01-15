module main;

   localparam int int_lparm = 11;
   parameter int  int_param = 10;
   int		  int_var;

   initial begin
      if (int_lparm != 11) begin
	 $display("FAILED: int_lparm=%b", int_lparm);
	 $finish;
      end

      if ($bits(int_lparm) != 32) begin
	 $display("FAILED: $bits(int_lparm) = %d", $bits(int_lparm));
	 $finish;
      end

      if (int_param != 10) begin
	 $display("FAILED: int_param=%b", int_param);
	 $finish;
      end

      if ($bits(int_param) != 32) begin
	 $display("FAILED: $bits(int_param) = %d", $bits(int_param));
	 $finish;
      end

      int_var = int_param;
      if (int_var != 10) begin
	 $display("FAILED: int_var=%b", int_var);
	 $finish;
      end

      $display("PASSED");
   end

endmodule // main
