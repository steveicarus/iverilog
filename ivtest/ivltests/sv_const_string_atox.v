module top;
  parameter string str_nav = "Not a value";
  parameter string str_isp = "1_024";
  parameter string str_imx = "4_294_967_295";
  parameter string str_iov = "4_294_967_297";
  parameter integer inav = str_nav.atoi();
  parameter integer isp  = str_isp.atoi();
  parameter integer imx  = str_imx.atoi();
  parameter integer iov  = str_iov.atoi();
  parameter string str_hsp = "400";
  parameter string str_hmx = "FFFF_FFFF";
  parameter string str_hov = "1_0000_0001";
  parameter integer hnav = str_nav.atohex();
  parameter integer hsp  = str_hsp.atohex();
  parameter integer hmx  = str_hmx.atohex();
  parameter integer hov  = str_hov.atohex();
  parameter string str_osp = "2_000";
  parameter string str_omx = "37_777_777_777";
  parameter string str_oov = "40_000_000_001";
  parameter integer onav = str_nav.atooct();
  parameter integer osp  = str_osp.atooct();
  parameter integer omx  = str_omx.atooct();
  parameter integer oov  = str_oov.atooct();
  parameter string str_bsp = "100_0000_0000";
  parameter string str_bmx = "11111111_11111111_11111111_11111111";
  parameter string str_bov = "1_00000000_00000000_00000000_00000001";
  parameter integer bnav = str_nav.atobin();
  parameter integer bsp  = str_bsp.atobin();
  parameter integer bmx  = str_bmx.atobin();
  parameter integer bov  = str_bov.atobin();
  reg passed;

  initial begin
    passed = 1'b1;

    // Check atoi.
    if (inav !== 0) begin
      $display("FAILED: Expected 0 (dec), got %0d", inav);
      passed = 1'b0;
    end

    if (isp !== 1024) begin
      $display("FAILED: Expected 1024 (dec), got %0d", isp);
      passed = 1'b0;
    end

    if (imx !== -1) begin
      $display("FAILED: Expected -1 (dec), got %0d", imx);
      passed = 1'b0;
    end

    if (iov !== 1) begin
      $display("FAILED: Expected 1 on overflow (dec), got %0d", iov);
      passed = 1'b0;
    end

    // Check atohex.
    if (hnav !== 0) begin
      $display("FAILED: Expected 0 (hex), got %0d", hnav);
      passed = 1'b0;
    end

    if (hsp !== 1024) begin
      $display("FAILED: Expected 1024 (hex), got %0d", hsp);
      passed = 1'b0;
    end

    if (hmx !== -1) begin
      $display("FAILED: Expected -1 (hex), got %0d", hmx);
      passed = 1'b0;
    end

    if (hov !== 1) begin
      $display("FAILED: Expected 1 on overflow (hex), got %0d", hov);
      passed = 1'b0;
    end

    // Check atooct.
    if (onav !== 0) begin
      $display("FAILED: Expected 0 (oct), got %0d", onav);
      passed = 1'b0;
    end

    if (osp !== 1024) begin
      $display("FAILED: Expected 1024 (oct), got %0d", osp);
      passed = 1'b0;
    end

    if (omx !== -1) begin
      $display("FAILED: Expected -1 (oct), got %0d", omx);
      passed = 1'b0;
    end

    if (oov !== 1) begin
      $display("FAILED: Expected 1 on overflow (oct), got %0d", oov);
      passed = 1'b0;
    end

    // Check atobin.
    if (bnav !== 0) begin
      $display("FAILED: Expected 0 (bin), got %0d", bnav);
      passed = 1'b0;
    end

    if (bsp !== 1024) begin
      $display("FAILED: Expected 1024 (bin), got %0d", bsp);
      passed = 1'b0;
    end

    if (bmx !== -1) begin
      $display("FAILED: Expected -1 (bin), got %0d", bmx);
      passed = 1'b0;
    end

    if (bov !== 1) begin
      $display("FAILED: Expected 1 on overflow (bin), got %0d", bov);
      passed = 1'b0;
    end

    if (passed) $display("PASSED");
  end
endmodule
