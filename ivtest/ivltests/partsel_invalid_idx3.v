// Check that a non-existent index into a vector results in an elaboration error.

module test;

  reg [31:0] a;
  wire [1:0] b;

  assign b = a[does_not_exist-:2]; // Error: Invalid base index

  initial begin
    $display("FAILED");
  end

endmodule
