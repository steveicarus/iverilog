// Check that a select of a packed array parameter can not use more indices
// than the parameter has dimensions.

module test;

  localparam logic [1:0][3:0] M = 8'h3e;

  initial begin
    $display("%h", M[0][1][0]);
  end

endmodule
