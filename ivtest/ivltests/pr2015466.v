// Copyright 2008, Martin Whitaker
// This file may be freely copied for any purpose. No attribution required.

module prXXX();

reg  [3:0] value1;
wire [1:0] value2;

assign value2 = (value1 == 0) ? 0 : (value1 == 1) ? 1 : 2;

   initial begin
      value1 = 0;
      #1 if (value2 !== 0) begin
	 $display("FAILED -- value1=%b, value2=%b", value1, value2);
	 $finish;
      end
      value1 = 1;
      #1 if (value2 !== 1) begin
	 $display("FAILED -- value1=%b, value2=%b", value1, value2);
	 $finish;
      end
      value1 = 2;
      #1 if (value2 !== 2) begin
	 $display("FAILED -- value1=%b, value2=%b", value1, value2);
	 $finish;
      end
      value1 = 3;
      #1 if (value2 !== 2) begin
	 $display("FAILED -- value1=%b, value2=%b", value1, value2);
	 $finish;
      end
      $display("PASSED");
      $finish;
   end // initial begin

endmodule
