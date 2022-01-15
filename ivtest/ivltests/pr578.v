module main;

   function [7:0] add;
      input [7:0] a, b;

      reg [8:0] tmp;
      begin
	 tmp = a + b;
	 if (tmp < 9'h100)
	   add = tmp;
	 else
	   add = 8'hff;
      end
   endfunction // add

   reg[7:0] out;
   initial begin

      out = 1? add(8,9) : 0;

      if (out !== 8'd17) begin
	 $display("FAILED -- out = %b", out);
	 $finish;
      end

      $display("PASSED");
   end

endmodule // main
