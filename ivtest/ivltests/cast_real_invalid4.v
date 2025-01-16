// Check that casting a dynamic array to real generates an error.

module test;

  real r;
  real d[];

  initial begin
    r = real'(d); // Error: Cast from dynamic array to real not allowed
  end

endmodule
