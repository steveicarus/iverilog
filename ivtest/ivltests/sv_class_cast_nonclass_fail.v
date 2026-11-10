// Check that $cast() of values that are not class handles is
// reported as not supported.

module test;

  typedef enum { RED, GREEN } color_t;

  color_t c;
  int rc;

  initial begin
    rc = $cast(c, 1);
    $display("FAILED");
  end

endmodule
