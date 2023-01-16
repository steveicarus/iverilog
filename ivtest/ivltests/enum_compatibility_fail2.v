// Check that an error is reported for implicit cast to enum when assigning
// to an array element.

module test;

  typedef enum reg [31:0] {
    A, B
  } E;

  E ea[2];

  initial begin
    ea[0] = 10; // This should fail. Implicit cast to enum.
    $display("FAILED");
  end

endmodule
