// Copyright 2008, Martin Whitaker.
// This file may be freely copied for any purpose.

module multiply();

reg signed [31:0] A;
reg signed [31:0] B;

wire signed [63:0] Y;

assign Y = A * B;

initial begin
  A = -1;
  B = -1;
  #1 $display("(%0d)*(%0d) = %0d", A, B, Y);
  if (Y !== 64'd1) begin
     $display("FAILED");
     $finish;
  end
  $display("PASSED");
end

endmodule
