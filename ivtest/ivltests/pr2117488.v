// Copyright 2008, Martin Whitaker.
// This file may be freely copied for any purpose.

module ternary_add();

reg        Enable;

reg  [7:0] A;
reg  [7:0] B;
reg        C;
wire [8:0] Y;

assign Y = Enable ? A + B + C : 0;

initial begin
  Enable = 1'b1;
  A = 8'd1;
  B = 8'd254;
  C = 1'd1;
  #1 $display("%0d + %0d + %0d = %0d", A, B, C, Y);
  if (Y !== 9'd256) begin
     $display("FAILED");
     $finish;
  end
  $display("PASSED");
end

endmodule
