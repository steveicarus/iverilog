// Check that casting a queue to real generates an error.

module test;

  real r;
  real q[$];

  initial begin
    r = real'(q); // Error: Cast from queue to real not allowed
  end

endmodule
