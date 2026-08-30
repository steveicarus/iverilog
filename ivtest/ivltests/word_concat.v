module word_concat;
  reg one_hi;
  reg one_lo;
  reg [6:0] part7;
  reg [24:0] part25;
  reg [15:0] part16_hi;
  reg [15:0] part16_lo;
  reg [30:0] part31;
  reg [31:0] part32;
  reg [32:0] part33;
  reg [62:0] part63;
  reg [63:0] part64;
  reg [1:0] result2;
  reg [31:0] result32;
  reg [63:0] result64;
  reg [64:0] result65;
  integer failed;

  task report_failure;
    input integer test_id;
    begin
      $display("FAILED test %0d", test_id);
      failed = failed + 1;
    end
  endtask

  initial begin
    failed = 0;
    one_hi = 1'bx;
    one_lo = 1'bz;
    part7 = 7'b1x0z101;
    part25 = 25'h12a55aa;
    part25[23] = 1'bx;
    part25[5] = 1'bz;
    part16_hi = 16'hx5a7;
    part16_lo = 16'hc3z1;
    part31 = 31'h5a5a5a5a;
    part31[17] = 1'bx;
    part32 = 32'h89abcdef;
    part32[28] = 1'bz;
    part33 = 33'h1_12345678;
    part33[7] = 1'bx;
    part63 = 63'h6123456789abcdef;
    part63[41] = 1'bz;
    part64 = 64'hfedcba9876543210;
    part64[52] = 1'bx;

    result2 = {one_hi, one_lo};
    if (result2[1] !== one_hi || result2[0] !== one_lo)
      report_failure(1);

    result32 = {part7, part25};
    if (result32[31:25] !== part7 || result32[24:0] !== part25)
      report_failure(2);

    result32 = {part16_hi, part16_lo};
    if (result32[31:16] !== part16_hi || result32[15:0] !== part16_lo)
      report_failure(3);

    result32 = {part7, 25'h155aa55};
    if (result32[31:25] !== part7 || result32[24:0] !== 25'h155aa55)
      report_failure(4);

    result64 = {part31, part33};
    if (result64[63:33] !== part31 || result64[32:0] !== part33)
      report_failure(5);

    result64 = {part32, 32'h13579bdf};
    if (result64[63:32] !== part32 || result64[31:0] !== 32'h13579bdf)
      report_failure(6);

    result64 = {part63, one_lo};
    if (result64[63:1] !== part63 || result64[0] !== one_lo)
      report_failure(7);

    result64 = {one_hi, part31, part32};
    if (result64[63] !== one_hi || result64[62:32] !== part31
        || result64[31:0] !== part32)
      report_failure(8);

    result65 = {one_hi, part64};
    if (result65[64] !== one_hi || result65[63:0] !== part64)
      report_failure(9);

    if (failed == 0)
      $display("PASSED");
  end
endmodule
