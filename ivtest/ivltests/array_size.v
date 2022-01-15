module test;

   parameter width = 16;
   localparam count = 1<<width;
   reg [width-1:0] array[count];

   integer	   idx;
   initial begin
      for (idx = 0 ; idx < count ; idx = idx+1)
	array[idx] = idx;

      if (array[count/2] !== count/2) begin
	 $display("FAILED");
	 $finish;
      end

      if (array[0] !== 0) begin
	 $display("FAILED");
	 $finish;
      end

      for (idx = 0 ; idx < count ; idx = idx+1)
	if (array[idx] !== idx) begin
	   $display("FAILED");
	   $finish;
	end

      $display("PASSED");
   end

endmodule
