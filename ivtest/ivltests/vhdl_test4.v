module test;

   wire [7:0] o1, o2, o3;
   dummy foo(.o1(o1), .o2(o2), .o3(o3));

   initial begin
      #1 if (o1 !== 8'h00) begin
	 $display("FAILED -- o1 = %b", o1);
	 $finish;
      end

      if (o2 !== 8'h08) begin
	 $display("FAILED -- o2 = %b", o2);
	 $finish;
      end

      if (o3 !== 8'h80) begin
	 $display("FAILED == o3 = %b", o3);
	 $finish;
      end

      $display("PASSED");
   end // initial begin

endmodule
