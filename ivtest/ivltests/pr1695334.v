module test();
   parameter     BITS = 4;
   parameter     C = 1;

   wire [BITS-1:0] a;
   reg [BITS-1:0]  a_bc;
   assign	   a =  a_bc - 2*C;

   initial begin
      a_bc = 9;
      #1 if (a !== 7) begin
	 $display("FAILED -- a_bc=%d, a=%d", a_bc, a);
	 $finish;
      end

      $display("PASSED");
   end

endmodule
