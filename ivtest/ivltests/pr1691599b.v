// pr1691599b.v
//
module main;

   parameter cnt = 4;

   genvar i;
   generate
      for (i = 0; i < cnt; i = i+1) begin : target
	 reg [1:0] val;
      end
   endgenerate

   initial begin
      target[0].val = 0;
      target[1].val = 1;
      target[2].val = 2;
      target[3].val = 3;
   end

   generate
      for (i = 0; i < cnt; i = i+1) begin : sink
	 wire [1:0] val = target[i].val;

	 initial #1
	   if (val !== i) begin
	      $display("FAILED: sink[%0d].val = %b", i, val);
	      $finish;
	   end
      end
   endgenerate

   initial #10 $display("PASSED");

endmodule // main
