// Test for GitHub issue #716
// Undimensioned array passed to task expecting single vector should error
module test();
  logic [7:0] mybuf [];

  task t1(output logic [7:0] buffer);
    buffer = 0;
  endtask

  initial begin
    t1(mybuf);
  end
endmodule
