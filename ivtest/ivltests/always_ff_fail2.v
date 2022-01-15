module top;
  reg q, clk, d;

  always_ff begin
    q <= d;
    @(posedge clk);
  end

  initial $display("Expected compile failure!");

endmodule
