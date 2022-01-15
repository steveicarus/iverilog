// pr1913918b

module test ( output a);

   parameter [9:1] b = 9'b0_0000_0010;

   assign a = b[1] ^ b[9];

   initial #1 begin
      if (a !== 1'b0) begin
	 $display("FAILED -- b=%b, a=%b", b, a);
	 $finish;
      end
      $display("PASSED");
   end

endmodule
