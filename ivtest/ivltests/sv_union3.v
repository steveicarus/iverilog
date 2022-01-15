
module main;

   typedef enum logic [3:0] { WORD0, WORD1, WORD9='b1001, WORDC='b1100 } word_t;

   typedef union packed {
      logic [3:0] bits;
      word_t words;
   } bits_t;

   bits_t foo;

   initial begin
      foo.bits = 'b1001;
      if (foo.bits !== 'b1001) begin
	 $display("FAILED -- foo.bits=%b", foo.bits);
	 $finish;
      end

      if (foo.words !== WORD9) begin
	 $display("FAILED -- foo.words=%b", foo.words);
	 $finish;
      end

      foo.words = WORDC;
      if (foo.words !== WORDC) begin
	 $display("FAILED -- foo.words=%b", foo.words);
	 $finish;
      end
      if (foo.bits !== 'b1100) begin
	 $display("FAILED -- foo.bits=%b", foo.bits);
	 $finish;
      end

      $display("PASSED");
   end

endmodule // main
