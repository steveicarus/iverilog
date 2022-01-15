module test();
   reg [1:0] array[1:0];
   reg	     sign;
   reg	     clk = 1'b0;

   always @(posedge clk)
     sign = array[1][1];

   initial begin
      array[0] = 1;
      array[1] = 2;

      #1 clk = 1;

      #1 if (sign !== 1'b1) begin
	 $display("FAILED -- array[1][1] = %b, sign=%b", array[1][1], sign);
	 $finish;
      end

      $display("PASSED");
   end

endmodule // test
