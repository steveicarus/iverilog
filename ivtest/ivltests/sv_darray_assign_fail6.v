// Check that it is not possible to assign a dynamic array with an int
// element type to a dynamic array with a real element type.

module test;

  real d1[];
  int d2[];

  initial begin
    d1 = d2;
    $display("FAILED");
  end

endmodule
