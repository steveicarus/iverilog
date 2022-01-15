module top;
  reg q, en, d;

  always_latch begin
    if (en) #0 q <= d;
  end

  initial $display("Expected compile failure!");

endmodule
