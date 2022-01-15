module main;

   reg [3:0] tmp;

   initial begin
      tmp = 10.7;

      if (tmp !== 4'd11) begin
	 $display("FAILED -- Incorrect rounding: tmp=%b", tmp);
	 $finish;
      end

      $display("PASSED");
   end

endmodule // main
