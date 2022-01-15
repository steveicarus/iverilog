module top;
  reg q, clk, d;

  always_ff @(posedge clk) begin
    #0 q <= d;
  end

  initial $display("Expected compile failure!");

endmodule
