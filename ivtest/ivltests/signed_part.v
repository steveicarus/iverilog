module top;
  reg pass = 1'b1;
  reg [14:-1] big = 16'h0123;

  reg signed [15:0] a;

  wire [3:0] w_big = big[a+:4];

  initial begin
    #1; // Wait for the assigns above to happen.

    /* If this fails it is likely because the index width is less
     * than an int width. */
    a = -2;
    #1;
    if (w_big !== 4'b011x) begin
      $display("Failed: .part/v check, expected 4'b011x, got %b.", w_big);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
