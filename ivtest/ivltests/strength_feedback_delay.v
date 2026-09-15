// Check that delayed gates settle when feedback leaves their value unchanged.

module test;

  wire en;
  wire b, n, p, c;

  bufif1 #1 (b, b, en);
  nmos #1 (n, n, en);
  pmos #1 (p, p, en);
  cmos #1 (c, c, en, en);

  initial begin
    #2;
    if ({b, n, p, c} === 4'bxxxx) begin
      $display("PASSED");
    end else begin
      $display("FAILED: expected xxxx, got %b", {b, n, p, c});
    end
  end

endmodule
