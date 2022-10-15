// Check that continuous assignment of two compatible arrays is supported

module test;

  wire [1:0] x[1:0];
  wire [1:0] y[1:0];

  assign x = y;

  initial begin
    $display("PASSED");
  end

endmodule
