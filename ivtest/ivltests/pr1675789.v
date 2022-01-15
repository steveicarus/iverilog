module main;

   parameter NIBBLE = 4;
   reg [NIBBLE*4-1:0] array [1:0];
   reg [3:0]  word;

   integer    idx;
   initial begin
      for (idx = 0 ; idx < 4 ;  idx = idx+1) begin
	 array[0][idx*NIBBLE +: 4] = +idx;
	 array[1][idx*NIBBLE +: 4] = -idx;
      end

      if (array[0] !== 16'h3210) begin
	 $display("FAILED -- array[0] = %h", array[0]);
	 $finish;
      end

      word = array[0][7:4];
      if (word !== 4'h1) begin
	 $display("FAILED == array[0][7:4] = %h", word);
	 $finish;
      end

      word = array[1][7:4];
      if (word !== 4'hf) begin
	 $display("FAILED == array[0][7:4] = %h", word);
	 $finish;
      end

      for (idx = 0 ;  idx < 4 ;  idx = idx+1) begin
	 word = array[0][idx*NIBBLE +: 4];
	 if (word !== idx) begin
	    $display("FAILED == array[0][nibble=%d] = %h", idx, word);
	    $finish;
	 end

	 word = array[1][idx*NIBBLE +: 4];
	 if (word !== - idx[3:0]) begin
	    $display("FAILED == array[1][nibble=%d] = %h", idx, word);
	    $finish;
	 end
      end // for (idx = 0 ;  idx < 4 ;  idx += 1)

      $display("PASSED");
   end

endmodule // main
