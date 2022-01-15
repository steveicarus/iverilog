/*
 * Test support for multi-dimension unpacked arrays.
 */
module main;

   localparam ISIZE = 6;
   localparam JSIZE = 8;
   reg [7:0] array[0:ISIZE-1][0:JSIZE-1];

   reg [3:0] idx, jdx;
   integer   count;
   initial begin
      // Load array contents.
      for (idx = 0 ; idx < ISIZE ; idx = idx+1) begin
	 for (jdx = 0 ; jdx < JSIZE ; jdx = jdx+1) begin
	    array[idx][jdx] = {idx, jdx};
	 end
      end

      if (array[3][2] !== 8'h32) begin
	 $display("FAILED -- array[3][2] == %h", array[3][2]);
	 $finish;
      end

      for (count = 0 ; count < 4096 ; count = count+1) begin
	 idx = {$random} % ISIZE;
	 jdx = {$random} % JSIZE;
	 if (array[idx][jdx] !== {idx,jdx}) begin
	    $display("FAILED -- array[%d][%d] == %h", idx, jdx, array[idx][jdx]);
	    $finish;
	 end
      end

      $display("PASSED");
      $finish;
   end // initial begin

endmodule // main
