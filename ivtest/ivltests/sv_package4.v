
// This tests SystemVerilog packages. Make sure that typedef
// names work.

package p1;
   typedef struct packed {
      bit [7:0] high;
      bit [7:0] low;
   } word_t;

   word_t word;
endpackage

module main;

   import p1::word;

   initial begin
      if ($bits(word) != 16) begin
	 $display("FAILED -- $bits(p1::word) == %0d", $bits(p1::word));
	 $finish;
      end

      word = 'haa55;

      if (word != 'haa55) begin
	 $display("FAILED -- p1::word = %h", word);
	 $finish;
      end

      word.low = 'h66;
      word.high = 'hbb;
      if (word != 'hbb66 || word.low != 8'h66) begin
	 $display("FAILED -- p1::word = %h", word);
	 $finish;
      end

      $display("PASSED");
      $finish;
   end // initial begin
endmodule // main
