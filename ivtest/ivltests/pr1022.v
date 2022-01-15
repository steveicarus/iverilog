// based on PR#1022

module foo;
   wire [-1:0] fred;
   assign      fred = 1;

   initial begin
      #1 if (fred[0] !== 1) begin
	 $display("FAILED -- fred[0] = %b", fred[0]);
	 $finish;
      end

      if (fred[-1] !== 0) begin
	 $display("FAILED -- fred[-1] = %b", fred[-1]);
	 $finish;
      end

      $display("PASSED");
   end // initial begin

endmodule
