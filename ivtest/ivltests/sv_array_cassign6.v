// Check that continuous array assignment behaves correctly when left-to-right
// order is reversed.

module test;

  wire [31:0] x[1:0];
  wire [31:0] y[0:1];

  assign x[0] = 1;
  assign x[1] = 2;
  assign y = x;

  final begin
    if (y[0] === 2 || y[1] === 1) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
