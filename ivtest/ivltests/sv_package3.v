
// This tests SystemVerilog packages. Make sure that typedef
// names work.

package p1;
   typedef struct packed {
      bit [7:0] high;
      bit [7:0] low;
   } word_t;
endpackage

program main;

   import p1::word_t;
   word_t word;

   initial begin
      if ($bits(word) != 16) begin
	 $display("FAILED -- $bits(word) == %0d", $bits(word));
	 $finish;
      end

      word.low = 'h55;
      word.high = 'haa;

      if (word != 'haa55) begin
	 $display("FAILED -- word = %h", word);
	 $finish;
      end

      $display("PASSED");
      $finish;
   end // initial begin
endprogram // main
