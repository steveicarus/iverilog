module top;
  reg q, clk, d;
  event foo;

  always_ff @(posedge clk) begin
    @foo q <= d;
  end

  initial $display("Expected compile failure!");

endmodule
