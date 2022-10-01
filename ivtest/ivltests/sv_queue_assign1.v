// Check that queues with compatible packed base types can be assigned to each
// other. Even if the element types are not identical.

module test;

  typedef bit [31:0] T1;
  typedef bit [31:0] T2[$];

  // For two packed types to be compatible they need to have the same packed
  // width, both be 2-state or 4-state and both be either signed or unsigned.
  bit [32:1] q1[$];
  bit [7:0][3:0] q2[$];
  int unsigned q3[$];
  T1 q4[$];
  T2 q5;

  initial begin
    q2 = q1;
    q3 = q2;
    q4 = q3;
    q5 = q4;
    q1 = q5;

    $display("PASSED");
  end

endmodule
