// Check that a genvar declared in a generate loop header can not be the target
// of a continuous assignment inside the loop.

module test;
  for (genvar i = 0; i < 2; i = i + 1) begin : g
    assign i = 1'b1; // Error: i is the loop localparam, not a net.
  end

  initial $display("FAILED");
endmodule
