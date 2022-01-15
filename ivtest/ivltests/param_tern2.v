/*
 * This example is a distillation of the essence of PR#993.
 * Or at least the essence that led to a bug report.
 */

module main;

   parameter [31:0] fifo_address = 32'hc0_00_00_00;

   reg [31:0]	    bar;
   reg		    flag;
   wire [31:0] foo  = flag? fifo_address  : bar;

   initial begin
      bar = ~fifo_address;

      flag = 1;
      #1 if (foo !== fifo_address) begin
	 $display("FAILED");
	 $finish;
      end

      flag = 0;
      #1 if (foo !== bar) begin
	 $display("FAILED");
	 $finish;
      end

      $display("PASSED");
   end // initial begin

endmodule // main
