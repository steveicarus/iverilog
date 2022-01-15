// Test behaviour when a multi-bit expression is used as the input of
// a singleton array of a primitive gate. The standard is explicit
// that this should be treated as an error.

module top;

reg  [1:0] in;
wire [2:0] out;

buf buf1[0:0](out[0], 1);
buf buf2[0:0](out[1], 2'b01);
buf buf3[0:0](out[2], in[1:0]);

initial begin
  in = 1;
  #1 $display("out = %b", out);
  // this should have failed at compile time
  $display("FAILED");
end

endmodule
