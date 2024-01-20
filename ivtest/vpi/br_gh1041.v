module test;
  wire w4;
  tri0 w8;
  wire real wr;

  reg failed = 0;

  initial begin
    #0;
    $display("w4 %b w8 %b wr %f", w4, w8, wr);
    if (w4 !== 1'b1) failed = 1;
    if (w8 !== 1'b1) failed = 1;
    if (wr != 1.0)   failed = 1;

    if (failed)
      $display("FAILED");
    else
      $display("PASSED");
  end
endmodule
