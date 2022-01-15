module top;
  reg q, en, d;
  event foo;

  always_latch begin
    if (en) @foo q <= d;
  end

  initial $display("Expected compile failure!");

endmodule
