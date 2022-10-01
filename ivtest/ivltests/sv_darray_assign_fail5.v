// Check that it is not possible to assign a dynamic array with a real
// element type to a dynamic array with an int element type.

module test;

  int d1[];
  real d2[];

  initial begin
    d1 = d2;
    $display("FAILED");
  end

endmodule
