// Check that it is an error to use a forwarded union type as the type for the
// members in the union itself.

module test;

  typedef T;

  typedef union packed {
    T x;
  } T;

  T x;

  initial begin
    $display("FAILED");
  end

endmodule
