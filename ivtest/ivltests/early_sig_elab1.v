// Icarus elaborates signals in alphabetical order, so force early
// elaboration that way.

module test;

localparam LSB = 0;
localparam MSB = 7;

reg [MSB:LSB] c;
reg [$bits(c):1] a;

localparam WIDTH = $bits(c);

reg [WIDTH:1] b;

initial begin
  $display("a = %b", a);
  $display("b = %b", b);
  if ($bits(a) === 8 && $bits(b) == 8)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
