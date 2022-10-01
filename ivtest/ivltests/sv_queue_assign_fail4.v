// Check that it is not possible to assign a queue with an enum element type to
// a queue with a packed type. Even if the enum base type is the same as the
// packed type.

module test;

  enum logic [31:0] {
    A
  } q1[$];
  logic [31:0] q2[$];

  initial begin
    q1 = q2;
    $dispaly("FAILED");
  end

endmodule
