// pr1784984
module signed_test;

   reg [31:0] a;

   initial begin
      a = (32'h80000000);
      a = a / 2;
      $display ("Current Value of a = %h", a);
      if (a !== 32'h40000000) begin
	 $display("FAILED");
	 $finish;
      end

      a = a * 2;
      $display("Current value of a = %h", a);
      if (a !== 32'h80000000) begin
	 $display("FAILED");
	 $finish;
      end

      a = (32'h80000000)/2;
      $display ("Current Value of a = %h", a);
      if (a !== 32'h40000000) begin
	 $display("FAILED");
	 $finish;
      end

      a = (32'h40000000)*2;
      $display ("Current Value of a = %h", a);
      if (a !== 32'h80000000) begin
	 $display("FAILED");
	 $finish;
      end

      $display("PASSED");
   end

endmodule // signed_test
