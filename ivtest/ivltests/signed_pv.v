`ifdef __ICARUS__
  `define SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
`endif

module top;
  parameter pval = -2;
  reg pass = 1'b1;
  reg [14:-1] big = 16'h0123;
  reg [15:0] big_0 = 16'h0123;
  reg signed [15:0] idxs [0:1];

  reg signed [15:0] a;

  reg [4*8:1] res;

  initial begin

    /* If this fails it is likely because the index width is less
     * than an integer width. */
    a = -2;
    $sformat(res, "%b", big[a+:4]);
    if (res !== "011x") begin
      $display("Failed: &PV<> check 1, expected 4'b011x, got %s.", res);
      pass = 1'b0;
    end

    a = 0;
    idxs[0] = -1;
    $sformat(res, "%b", big_0[idxs[a]+:4]);
    if (res !== "011x") begin
      $display("Failed: &PV<> check 2, expected 4'b011x, got %s.", res);
      pass = 1'b0;
    end

    /* This should work since it is a constant value. */
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    $sformat(res, "%b", big[pval+:4]);
`else
    $sformat(res, "%bx", big[(pval+1)+:3]);
`endif
    if (res !== "011x") begin
      $display("Failed: &PV<> check 3, expected 4'b011x, got %s.", res);
      pass = 1'b0;
    end

    /* This should always work since it uses the index directly. */
    a = -1;
    $sformat(res, "%b", big_0[a+:4]);
    if (res !== "011x") begin
      $display("Failed: &PV<> check 4, expected 4'b011x, got %s.", res);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
