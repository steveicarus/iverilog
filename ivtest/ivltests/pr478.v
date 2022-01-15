/*
 * This is from iverilog issue # 1313453
 * This point is that it should compile.
 */
module test `protect (
a
);

// Input Declarations
input a;
`endprotect

initial $display("PASSED");

endmodule
