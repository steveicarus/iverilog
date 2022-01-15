module main;

   // Declare word1 as a VARIABLE
   struct packed {
      logic [7:0] high;
      logic [7:0] low;
   } word1;

   // Declare word2 as a VARIABLE
   struct packed {
      logic [7:0] high;
      logic [7:0] low;
   } word2;

   always @(word1) begin
      word2.high = word1.high;
      word2.low  = word1.low;
   end

   initial begin
      word1 = 16'haa_55;
      if (word1.high !== 8'haa || word1.low !== 8'h55) begin
	 $display("FAILED: word1 = %h, word1.high = %h, word1.low = %h",
		  word1, word1.high, word1.low);
	 $finish;
      end

      #1 /* Make sure word2 assign propagates */;

      if (word2.high !== 8'haa || word2.low !== 8'h55) begin
	 $display("FAILED: word2 = %h, word2.high = %h, word2.low = %h",
		  word1, word2.high, word2.low);
	 $finish;
      end

      $display("PASSED");
   end

endmodule // main
