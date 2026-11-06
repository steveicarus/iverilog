// Check that a default value that is not itself an assignment pattern is
// reported as unsupported for a multi-dimensional unpacked array.

module test;

  int a[2][3];

  initial begin
    a = '{default: 1};
  end

endmodule
