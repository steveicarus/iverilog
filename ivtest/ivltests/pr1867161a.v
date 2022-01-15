module test;
  /* The base+b calculation uses %load/vp0 and this will cause invalid
   * results when the sum of base+b is larger than what will fit
   * in b. The addition is done at b's width. It appears that
   * %load/vp0 needs to be enhanced, or something else needs to
   * be used.
   *
   * The workaround is to make b large enough to access
   * the largest a index not a's range. */
  parameter base = 8;
  reg [31:0] a[15:base];
  reg [2:0]  b;

  initial begin
    for (b=0; b<7; b=b+1) begin
      a[base+b] = 32'd2+b;
      $display("Value[%0d]: %1d", b, a[base+b]);
    end
  end
endmodule
