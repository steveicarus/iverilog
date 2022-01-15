module top;
  wire [15:0] number =16'h20;
  wire shift_cmp = (number == (1 << 5));

  initial begin
    #1; // Make sure things are settled.
    if (shift_cmp === 1'b1) $display("PASSED");
    else $display("FAILED, got %b expected 1'b1", shift_cmp);
  end
endmodule
