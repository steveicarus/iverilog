
typedef struct packed {
   logic [1:0] hig;
   logic [1:0] low;
} word_as_nibbles;

typedef union packed {
   logic [3:0] bits;
   word_as_nibbles words;
} bits_t;

module main;

   bits_t foo;

   initial begin
      foo.bits = 'b1001;
      if (foo.bits != 'b1001) begin
	 $display("FAILED -- foo.bits=%b", foo.bits);
	 $finish;
      end

      if (foo.words != 'b1001) begin
	 $display("FAILED -- foo.words=%b", foo.words);
	 $finish;
      end

      //foo.words.low = 'b00;
      //foo.words.hig = 'b11;
      foo.words = 'b1100;
      if (foo.words != 'b1100) begin
	 $display("FAILED -- foo.words=%b", foo.words);
	 $finish;
      end
      if (foo.bits != 'b1100) begin
	 $display("FAILED -- foo.bits=%b", foo.bits);
	 $finish;
      end

      $display("PASSED");
   end

endmodule // main
