// Check behaviour with variable array indices on LHS of procedural
// continuous (reg) assignment. This should be rejected by the compiler.
module top;

reg array1[2:1];

integer index = 1;

initial begin
  assign array1[index] = 1'b1;
  deassign array1[index];
end

endmodule
