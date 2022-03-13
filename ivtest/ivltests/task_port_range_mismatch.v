// Check that range mismatches between port direction and data type are detected
// for task ports. An error should be reported and no crash should occur.

module test;

  task t;
    input [1:0] x;
    reg [3:0] x;
    reg [3:0] y;
    y = x;
    $display("FAILED");
  endtask

  initial begin
    t(4'b1001);
  end

endmodule
