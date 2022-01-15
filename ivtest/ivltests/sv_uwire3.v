
// output ports may be uwire, or even a variable, if wire-ness
// or variable-ness are not explicitly stated.

typedef struct packed {
   logic [1:0] a;
   logic [1:0] b;
} sample_t;

module main;
   sample_t dst;
   logic [1:0] src_a, src_b;

   DUT dut(.out(dst), .x(src_a), .y(src_b));

   initial begin
      src_a = 1;
      src_b = 2;

      #1 /* wait for dst */;
      if (dst.a !== 1) begin
	 $display("FAILED -- dst.a=%b (dst=%b)", dst.a, dst);
	 $finish;
      end

      if (dst.b !== 2) begin
	 $display("FAILED -- dst.b=%b (dst=%b)", dst.b, dst);
	 $finish;
      end

      $display("PASSED");
   end
endmodule // main

module DUT(output sample_t out,
	   input logic [1:0] x, y);

   always @* begin
      out.a = x;
      out.b = y;
   end

endmodule
