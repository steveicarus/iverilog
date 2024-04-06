// Strictly speaking this is not legal as it uses a hierarchical name in a
// constant expression,

module test1;

reg [$bits(test2.v):1] v;

endmodule

module test2;

reg [7:0] v;

initial begin
  if ($bits(test1.v) === 8 && $bits(test3.v) === 8)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule

module test3;

reg [$bits(test2.v):1] v;

endmodule
