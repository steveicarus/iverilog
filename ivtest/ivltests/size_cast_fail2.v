// Check that an error is reported when using a negative value for a size cast.

module test;

  localparam integer N = -1;

  initial begin
    int x, y;
    y = N'(x); // This should fail, N is negative
  end

endmodule
