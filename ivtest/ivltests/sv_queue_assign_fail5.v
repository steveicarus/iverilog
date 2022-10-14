// Check that it is not possible to assign a queue with a real element type to a
// queue with an int element type.

module test;

  int q1[$];
  real q2[$];

  initial begin
    q1 = q2;
    $display("FAILED");
  end

endmodule
