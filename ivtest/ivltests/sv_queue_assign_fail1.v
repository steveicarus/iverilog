// Check that it is not possible to assign a queue with a 2-state element type
// to a queue with 4-state element type. Even if they are otherwise identical.

module test;

  logic [31:0] q1[$];
  bit [31:0] q2[$];

  initial begin
    q1 = q2;
    $dispaly("FAILED");
  end

endmodule
