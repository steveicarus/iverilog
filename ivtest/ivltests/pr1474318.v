module test;
   wire [5:0] a;
   reg [3:0]  b;
   assign     a[1:0] = {{2'b0},b};

   initial begin
      b = 4'b0110;
      #1 if (a !== 6'bzzzz10) begin
	 $display("FAILED -- a=%b", a);
	 $finish;
      end
      $display("PASSED");
   end

endmodule
