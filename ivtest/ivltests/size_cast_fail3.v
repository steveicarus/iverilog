// Check that an error is reported when using an undefined value for a size cast.

module test;

  localparam integer N = 32'hx;

  initial begin
    int x, y;
    y = N'(x); // This should fail, N is undefined
  end

endmodule
