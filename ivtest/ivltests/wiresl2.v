`begin_keywords "1364-2005"
/*
 * This test is from PR#193
 */

/* test:tshl
 Compilation fails with vvp from icarus verilog-20010616 snapshot

$ iverilog -t vvp tshl.v
ivl: eval_expr.c:418: draw_binary_expr_ls: Assertion `0' failed.


In vvm, runtime has trouble with $display

$ iverilog tshl.v
$ ./a.out
out=01<some binary characters>

(looks like the  correct output "out=01" followed by some
 random memory garbage.)

 */

module tshl;

   reg [2:0] bit;
   wire [7:0] shbit;
   integer    i;


   shl shl_0(shbit, bit);

   initial begin

      for(i = 0; i < 8; i = i + 1) begin
	 bit <= i[2:0];
	 #1
	    $display("out=%h", shbit);
      end // for (i = 0; i < 8; i = i + 1)

      $finish(0);
   end // initial begin
endmodule

module shl(out, bit);

   output [7:0] out;
   input [2:0]	bit;

   reg [7:0]	out_reg;

   always @(bit) begin
      out_reg <= 8'h01 << bit;

   end // always @ (bit)

   assign out = out_reg;

endmodule // shl
`end_keywords
