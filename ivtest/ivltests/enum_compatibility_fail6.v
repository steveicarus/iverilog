// Check that an error is reported for implicit cast to enum when assigning to
// struct members.

module test;

  typedef enum reg [31:0] {
    A, B
  } E;

  struct packed {
    E e;
  } s;

  initial begin
    s.e = 10; // This should fail. Implicit cast to enum.
    $display("FAILED");
  end

endmodule
