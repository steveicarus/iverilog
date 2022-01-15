// Test behaviour when a multi-bit expression is used as the input of
// a single instance of a primitive gate. The standard is quiet about
// this, but the consensus among other simulators is that the LSB of
// the expression is used.

module top;

reg  [1:0] in;
wire [2:0] out;

buf buf1(out[0], 1);
buf buf2(out[1], 2'b01);
buf buf3(out[2], in[1:0]);

initial begin
  in = 1;
  #1 $display("out = %b", out);
  if (out === 3'b111)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
