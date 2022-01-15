module top;
  // Does this have to be signed?
  wire signed [7:0] out;

  // Either of these will not expand correctly.
//  assign out = 'sh1f;
  assign out = 5'sh1f;

  initial #1 $displayb(out);
endmodule
