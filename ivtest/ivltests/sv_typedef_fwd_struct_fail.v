// Check that it is an error to use a forwarded struct type as the type for the
// member in the struct itself.

module test;

  typedef T;

  typedef struct packed {
    T x;
  } T;

  T x;

  initial begin
    $display("FAILED");
  end

endmodule
