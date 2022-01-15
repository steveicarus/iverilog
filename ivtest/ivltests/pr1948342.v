module test;

   wire        a;
   reg [20:0]  b;

   assign a = ((b[20:4]) || (b[3] && b[2:0])) ? 1'b0 : 1'b1;

   initial begin
      b = 0;
      #1 if (a !== 1'b1) begin
	 $display("FAILED -- b=%h, a=%b", b, a);
	 $finish;
      end

      b = 8;
      #1 if (a !== 1'b1) begin
	 $display("FAILED -- b=%h, a=%b", b, a);
	 $finish;
      end

      b = 12;
      #1 if (a !== 1'b0) begin
	 $display("FAILED -- b=%h, a=%b", b, a);
	 $finish;
      end

      b = 16;
      #1 if (a !== 1'b0) begin
	 $display("FAILED -- b=%h, a=%b", b, a);
	 $finish;
      end

      $display("PASSED");
   end

endmodule
