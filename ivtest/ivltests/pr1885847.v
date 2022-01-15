// Copyright 2008, Martin Whitaker.
// This file may be freely copied for any purpose.

module localparam_sign();

localparam	P1	= 16;
localparam	P2	= P1 * 2;

submodule #(P2) submodule();

endmodule

module submodule();

parameter	P3	= 1;

initial begin
  $display("P3 = %0d", P3);
end

endmodule
