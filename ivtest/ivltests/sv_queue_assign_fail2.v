// Check that it is not possible to assign a queue with a signed element type to
// a queue with a unsigned element type. Even if they are otherwise identical.

module test;

  logic [31:0] q1[$];
  logic signed [31:0] q2[$];

  initial begin
    q1 = q2;
    $dispaly("FAILED");
  end

endmodule
