module Test ( );
   wire w1;
   wire w2;
   reg array_of_reg [3:0];

   // FAIL
   wire ww = array_of_reg[0] == 1 ? 1 : 0;

   // workaround
   assign w1 = array_of_reg[0];
   assign w2 = w1 == 1 ? 1 : 0;

   initial begin
      array_of_reg[0] = 1'b0;
      array_of_reg[3] = 1'b1;

      #10 $display("ww=%b, w1=%b, w2=%b", ww, w1, w2);
      if (ww !== w2) begin
	 $display("FAILED -- ww !== w2");
	 $finish;
      end

      $display("PASSED");
   end

endmodule
