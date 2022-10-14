// Check that it is not possible to assign a dynamic array with a 2-state
// element type to a dynamic array with 4-state element type. Even if they are
// otherwise identical.

module test;

  logic [31:0] d1[];
  bit [31:0] d2[];

  initial begin
    d1 = d2;
    $dispaly("FAILED");
  end

endmodule
