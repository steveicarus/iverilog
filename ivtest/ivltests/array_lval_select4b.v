// Check behaviour with variable array indices on LHS of procedural
// continuous (net) assignment. This should be rejected by the compiler.
module top;

wire array1[2:1];

integer index = 1;

initial begin
  force array1[index] = 1'b1;
  release array1[index];
end

endmodule
