// Check that an error is reported for implicit cast to enum when assigning
// from an array element.

module test;

  typedef enum reg [31:0] {
    A, B
  } E;

  E e;
  int ea[2];

  initial begin
    ea[0] = B; // This is OK.
    e = ea[0]; // This should fail. Implicit cast to enum.
    $display("FAILED");
  end

endmodule
