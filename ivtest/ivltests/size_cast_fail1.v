// Check that an error is reported when using a zero value for a size cast.

module test;

  localparam integer N = 0;

  initial begin
    int x, y;
    y = N'(x); // This should fail, N is zero
  end

endmodule
