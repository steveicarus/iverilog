// Check that keyed patterns for dynamic arrays are reported as unsupported.

module test;

  int a[];

  initial begin
    a = '{0: 1, default: 0};
  end

endmodule
