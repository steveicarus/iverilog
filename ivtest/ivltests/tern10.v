module main;

   reg flag;
   reg [3:0] a, b;
   wire [4:0] Q = flag? a : b;

   initial begin
      flag = 1;
      a = 4'b1010;
      b = 4'b0101;

      #1 $display("%b = %b? %b : %b", Q, flag, a, b);

      if (Q !== 5'b01010) begin
	 $display("FAILED -- Q=%b, flag=%b, a=%b", Q, flag, a);
	 $finish;
      end

      flag = 0;
      #1 if (Q !== 5'b00101) begin
	 $display("FAILED -- Q=%b, flag=%b, b=%b", Q, flag, b);
	 $finish;
      end

      $display("PASSED");
   end

endmodule // main
