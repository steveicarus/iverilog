module main;

   reg clk;

   localparam integer TEST = 100;
   print #("PASSED", TEST) foo (clk);

   initial begin
      clk = 0;
      #1 clk = 1;
      #1 $finish;
   end

endmodule // main

module print (input wire clk);

   parameter message = "";
   parameter number = 0;

   always @(posedge clk) begin
      if (number !== 100) begin
	 $display("FAILED -- number=%d\n", number);
	 $finish;
      end
     $display("%s", message);
   end

endmodule // print
