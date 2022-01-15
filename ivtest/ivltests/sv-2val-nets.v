// This tests part selects of 2-value logic vectors through
// module ports. This is not supported in SystemVerilog, but
// we expect it to work as an Icarus Verilog extension, as
// long as all the bits of the 2-value are singly driven.

module main;

   bit [5:0] a, b;
   wire bit [6:0] sum;
   wire bit c2, c4;

   sub b10 (.c_i(1'b0), .a(a[1:0]), .b(b[1:0]), .out(sum[1:0]), .c_o(c2));
   sub b32 (.c_i(c2),   .a(a[3:2]), .b(b[3:2]), .out(sum[3:2]), .c_o(c4));
   sub b54 (.c_i(c4),   .a(a[5:4]), .b(b[5:4]), .out(sum[5:4]), .c_o(sum[6]));

   bit [6:0] idxa, idxb;
   initial begin
      for (idxa = 0 ; idxa < 'b1_000000 ; idxa = idxa+1) begin
	 for (idxb = 0 ; idxb < 'b1_000000 ; idxb = idxb+1) begin
	    a = idxa;
	    b = idxb;
	    #1 /* wait for devices to settle */;
	    if (idxa + idxb != sum) begin
	       $display("FAILED: %0d + %0d --> %0d", a, b, sum);
	       $stop;
	    end
	 end
      end // for (idxa = 0 ; idxa < 'b1_000000 ; idxa = idxa+1)
      $display("PASSED");
      $finish;
   end // initial begin

endmodule // main

module sub (input wire bit c_i, input wire bit[1:0] a, b,
	    output wire bit [1:0] out, output wire bit c_o);

   assign {c_o, out} = {1'b0, a} + {1'b0, b} + {2'b00, c_i};

endmodule // sub
