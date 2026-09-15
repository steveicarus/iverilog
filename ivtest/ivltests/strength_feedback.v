// Check that stable feedback through three-state gates stops propagating.

module test;

  wire en;
  wire b0, b1, n0, n1;

  bufif0 (b0, b0, en);
  bufif1 (b1, b1, en);
  notif0 (weak0, weak1) (n0, n0, en);
  notif1 (weak0, weak1) (n1, n1, en);

  initial begin
    #1;
    if ({b0, b1, n0, n1} === 4'bxxxx) begin
      $display("PASSED");
    end else begin
      $display("FAILED: expected xxxx, got %b", {b0, b1, n0, n1});
    end
  end

endmodule
