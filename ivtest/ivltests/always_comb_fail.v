module top;
  reg q, d;

  always_comb begin
    #0 q = d;
  end

  initial $display("Expected compile failure!");

endmodule
