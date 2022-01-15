module test;

reg [31:0] vec;

initial begin
  vec = 0;  // make sure vec is not pruned
  $test;
end

endmodule
