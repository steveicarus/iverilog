module top;
  reg q, d;
  event foo;

  always_comb begin
    @foo q = d;
  end

  initial $display("Expected compile failure!");

endmodule
