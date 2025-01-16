// Check that casting a string to real generates an error.

module test;

  real r;
  string s;

  initial begin
    r = real'(s); // Error: Cast from string to real not allowed
  end

endmodule
