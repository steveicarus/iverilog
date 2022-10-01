// Check that queues with compatible packed base types can be passed as task
// arguments. Even it the element types are not identical.

module test;

  typedef logic [31:0] T[$];

  task t1(logic [31:0] q[$]);
    q[0] = 1;
  endtask

  task t2(logic [7:0][3:0] q[$]);
    q[0] = 1;
  endtask

  task t3([31:0] q[$]);
    q[0] = 1;
  endtask

  task t4(T q);
    q[0] = 1;
  endtask

  // For two packed types to be compatible they need to have the same packed
  // width, both be 2-state or 4-state and both be either signed or unsigned.
  logic [31:0] q1[$];
  logic [7:0][3:0] q2[$];

  initial begin
    q1.push_back(1);
    q2.push_back(2);

    t1(q1);
    t1(q2);
    t2(q1);
    t2(q2);
    t3(q1);
    t3(q2);
    t4(q1);
    t4(q2);

    $display("PASSED");
  end

endmodule
