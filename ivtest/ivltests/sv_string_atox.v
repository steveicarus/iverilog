module top;
  integer res;
  string str;
  reg passed;

  initial begin
    passed = 1'b1;

    // Check atoi.
    str = "Not a value";
    res = str.atoi();
    if (res !== 0) begin
      $display("FAILED: Expected 0 (dec), got %0d", res);
      passed = 1'b0;
    end

    str = "1_024";
    res = str.atoi();
    if (res !== 1024) begin
      $display("FAILED: Expected 1024 (dec), got %0d", res);
      passed = 1'b0;
    end

    str = "4_294_967_295";
    res = str.atoi();
    if (res !== -1) begin
      $display("FAILED: Expected -1 (dec), got %0d", res);
      passed = 1'b0;
    end

    str = "4_294_967_297";
    res = str.atoi();
    if (res !== 1) begin
      $display("FAILED: Expected 1 on overflow (dec), got %0d", res);
      passed = 1'b0;
    end

    // Check atohex.
    str = "Not a value";
    res = str.atohex();
    if (res !== 0) begin
      $display("FAILED: Expected 0 (hex), got %0d", res);
      passed = 1'b0;
    end

    str = "400";
    res = str.atohex();
    if (res !== 1024) begin
      $display("FAILED: Expected 1024 (hex), got %0d", res);
      passed = 1'b0;
    end

    str = "FFFF_FFFF";
    res = str.atohex();
    if (res !== -1) begin
      $display("FAILED: Expected -1 (hex), got %0d", res);
      passed = 1'b0;
    end

    str = "1_0000_0001";
    res = str.atohex();
    if (res !== 1) begin
      $display("FAILED: Expected 1 on overflow (hex), got %0d", res);
      passed = 1'b0;
    end

    // Check atooct.
    str = "Not a value";
    res = str.atooct();
    if (res !== 0) begin
      $display("FAILED: Expected 0 (oct), got %0d", res);
      passed = 1'b0;
    end

    str = "2000";
    res = str.atooct();
    if (res !== 1024) begin
      $display("FAILED: Expected 1024 (oct), got %0d", res);
      passed = 1'b0;
    end

    str = "37_777_777_777";
    res = str.atooct();
    if (res !== -1) begin
      $display("FAILED: Expected -1 (oct), got %0d", res);
      passed = 1'b0;
    end

    str = "40_000_000_001";
    res = str.atooct();
    if (res !== 1) begin
      $display("FAILED: Expected 1 on overflow (oct), got %0d", res);
      passed = 1'b0;
    end

    // Check atobin.
    str = "Not a value";
    res = str.atobin();
    if (res !== 0) begin
      $display("FAILED: Expected 0 (bin), got %0d", res);
      passed = 1'b0;
    end

    str = "10000000000";
    res = str.atobin();
    if (res !== 1024) begin
      $display("FAILED: Expected 1024 (bin), got %0d", res);
      passed = 1'b0;
    end

    str = "11111111_11111111_11111111_11111111";
    res = str.atobin();
    if (res !== -1) begin
      $display("FAILED: Expected -1 (bin), got %0d", res);
      passed = 1'b0;
    end

    str = "40_000_000_001";
    str = "1_00000000_00000000_00000000_00000001";
    res = str.atobin();
    if (res !== 1) begin
      $display("FAILED: Expected 1 on overflow (bin), got %0d", res);
      passed = 1'b0;
    end

    if (passed) $display("PASSED");
  end
endmodule
