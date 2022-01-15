module top;
  wire out;
//  wire in; // Adding this makes it compile.
  assign out = ~in;
  zero zzz(in);
  initial #1 if (out == 1'b1) $display("PASSED"); else $display("FAILED");
endmodule

module zero(output wire foo);
   assign foo = 1'b0;
endmodule
