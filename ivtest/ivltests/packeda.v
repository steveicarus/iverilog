module top;

   // packed 2D array, arranged as 4 bytes of 8 bits each.
   logic [3:0][7:0] word32;
   int		    idx;
   int		    x;

   // Show a slice select in a continuous assignment
   wire [7:0]	    word1 = word32[1];

   initial begin
      // Const slice select in l-values.
      word32[0] = 'h00;
      word32[1] = 'h11;
      word32[2] = 'h22;
      word32[3] = 'h33;

      if (word32 !== 'h33_22_11_00) begin
	 $display("FAILED -- word32 = %h (1)", word32);
	 $finish;
      end

      #1 if (word1 !== 8'h11) begin
	 $display("FAILED -- word1 = %h", word1);
	 $finish;
      end

      // Non-constant slice indices, l-value and r-value.
      for (idx = 0 ; idx < 4 ; idx = idx+1)
	word32[idx] = ~word32[idx];

      if (word32 !== ~ 'h33_22_11_00) begin
	 $display("FAILED -- word32 = %h (2)", word32);
	 $finish;
      end

      word32[0][3:0] = 'h0;
      word32[1][3:0] = 'h1;
      word32[2][3:0] = 'h2;
      word32[3][3:0] = 'h3;

      word32[0][7:4] = 'h3;
      word32[1][7:4] = 'h2;
      word32[2][4 +: 4] = 'h1;
      word32[3][4 +: 4] = 'h0;

      if (word32 !== 'h03_12_21_30) begin
	 $display("FAILED -- word32 = %h (3)", word32);
	 $finish;
      end

      if (word32[1][7:4] !== word32[1][4 +: 4]) begin
	 $display("FAILED -- word32[1][7:4]=%h, word32[1][4 +: 4]=%h",
		  word32[1][7:4],word32[1][4 +: 4]);
	 $finish;
      end

      x = 4;
      word32[1][x +: 4] = 'h2;
      if (word32[1][7:4] !== word32[1][x +: 4]) begin
	 $display("FAILED -- word32[1][7:4]=%h, word32[1][4 +: 4]=%h",
		  word32[1][7:4],word32[1][x +: 4]);
	 $finish;
      end

      for (idx = 0 ; idx < 8 ; idx = idx+1) begin
	 word32[0][idx] = idx[0];
	 word32[2][idx] = idx[0];
	 word32[1][idx] = ~idx[0];
	 word32[3][idx] = ~idx[0];
      end

      if (word32 !== 'h55_aa_55_aa) begin
	 $display("FAILED -- word32 = %h (4)", word32);
	 $finish;
      end

      for (idx = 0 ; idx < 8 ; idx = idx+1) begin
	 if (word32[0][idx] !== word32[2][idx]) begin
	    $display("FAILED -- word32[0][%0d]=%b, word32[2][%0d]=%b",
		     idx, word32[0][idx], idx, word32[2][idx]);
	    $finish;
	 end
	 if (word32[1][idx] !== word32[3][idx]) begin
	    $display("FAILED -- word32[1][%0d]=%b, word32[3][%0d]=%b",
		     idx, word32[1][idx], idx, word32[3][idx]);
	    $display("FAILED");
	    $finish;
	 end
      end

      $display("PASSED");
   end

endmodule
