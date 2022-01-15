module top;
  reg signed [63:0] a = 64'h12345678abcdabcd;
  reg signed [63:0] b = 64'h1234;

  initial
    if (a/b !== 64'h10004c017806b)
      $display("FAILED: expected 64'h10004c017896b, got 64'h%h", a/b);
    else $display("PASSED");
endmodule
