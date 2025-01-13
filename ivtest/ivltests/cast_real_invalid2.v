// Check that casting a array to real generates an error.

module test;

  real r;
  real a[10];

  initial begin
    r = real'(a); // Error: Cast from array to real not allowed
  end

endmodule
