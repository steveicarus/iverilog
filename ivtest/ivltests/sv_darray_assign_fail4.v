// Check that it is not possible to assign a dynamic array with an enum
// element type to a dynamic array with a packed type. Even if the enum base
// type is the same as the packed type.

module test;

  enum logic [31:0] {
    A
  } d1[];
  logic [31:0] d2[];

  initial begin
    d1 = d2;
    $dispaly("FAILED");
  end

endmodule
