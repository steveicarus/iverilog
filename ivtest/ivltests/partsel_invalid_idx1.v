// Check that a non-existent index into a vector results in an elaboration error.

module test;

  reg [31:0] a;
  wire b;

  assign b = a[does_not_exist]; // Error: Invalid index

  initial begin
    $display("FAILED");
  end

endmodule
