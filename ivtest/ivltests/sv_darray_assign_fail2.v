// Check that it is not possible to assign a dynamic array with a signed element
// type to a dynamic array with a unsigned element type. Even if they are
// otherwise identical.

module test;

  logic [31:0] d1[];
  logic signed [31:0] d2[];

  initial begin
    d1 = d2;
    $dispaly("FAILED");
  end

endmodule
