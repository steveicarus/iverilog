// Check that range mismatches between port direction and data type are detected
// for module ports. An error should be reported and no crash should occur.

module test;
  input [1:0] x;
  wire [3:0] x;

  wire [3:0] y;

  assign y = x;

  initial begin
    $display("FAILED");
  end

endmodule
