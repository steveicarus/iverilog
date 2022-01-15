// Copyright 2008, Martin Whitaker
// This file may be freely copied for any purpose

module shift();

reg [5:0] S;
wire [63:0] Y;

assign Y = 1 << S;

initial begin
  S = 32;
  #1 $display("1 << %0d = %b", S, Y);
end

endmodule
