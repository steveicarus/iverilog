/*
 * This test reflects the problem reported in PR#1115.
 */
module test;

   reg[7:0] addr [0:2], pixel;

   wire     match0 = addr[0] == pixel;

   initial begin
      pixel = 1;
      addr[0] = 1;
      addr[1] = 2;
      addr[2] = 3;
      #1 if (match0 !== 1'b1) begin
	 $display("FAILED -- match0 is %b", match0);
	 $finish;
      end

      $display("PASSED");
      $finish;
   end // initial begin

endmodule
