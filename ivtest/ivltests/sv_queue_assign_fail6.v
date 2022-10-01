// Check that it is not possible to assign a queue with an int element type to a
// queue with a real element type.

module test;

  real q1[$];
  int q2[$];

  initial begin
    q1 = q2;
    $display("FAILED");
  end

endmodule
