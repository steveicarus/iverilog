// Check that it is not possible to assign a queue to another queue with a
// different element width.

module test;

  int q1[$];
  shortint q2[$];

  initial begin
    q1 = q2;
    $dispaly("FAILED");
  end

endmodule
