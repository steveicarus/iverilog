module top;
  reg [31:0] in_32, ck_32, out_32, out_32x;
  reg [63:0] out_64;
  integer res, fd;
  reg passed;

  initial begin
    passed = 1'b1;

    // Check that a normal 32 bit %u catches missing bytes.
    fd = $fopen("work/test_fscanf.bin", "wb");
    in_32 = 32'b000x100z_001z000x_101xxxzz_100z111x;
    $fwrite(fd, "%c%c", in_32[15:8], in_32[7:0]);
    $fclose(fd);
    fd = $fopen("work/test_fscanf.bin", "rb");
    res = $fscanf(fd, "%u", out_32);
    if (res !== -1) begin
      $display("FAILED: $fscanf() #1 returned %d (%b)", res, out_32);
      passed = 1'b0;
    end
    $fclose(fd);

    // Check that a normal 64 bit %u catches missing bytes (1/4).
    fd = $fopen("work/test_fscanf.bin", "wb");
    in_32 = 32'b000x100z_001z000x_101xxxzz_100z111x;
    $fwrite(fd, "%c%c", in_32[15:8], in_32[7:0]);
    $fclose(fd);
    fd = $fopen("work/test_fscanf.bin", "rb");
    res = $fscanf(fd, "%u", out_64);
    if (res !== -1) begin
      $display("FAILED: $fscanf() #2a returned %d (%b)", res, out_64);
      passed = 1'b0;
    end
    $fclose(fd);

    // Check that a normal 64 bit %u catches missing bytes (1/2).
    fd = $fopen("work/test_fscanf.bin", "wb");
    in_32 = 32'b000x100z_001z000x_101xxxzz_100z111x;
    $fwrite(fd, "%u", in_32);
    $fclose(fd);
    fd = $fopen("work/test_fscanf.bin", "rb");
    res = $fscanf(fd, "%u", out_64);
    if (res !== -1) begin
      $display("FAILED: $fscanf() #2b returned %d (%b)", res, out_64);
      passed = 1'b0;
    end
    $fclose(fd);

    // Check that a normal 64 bit %u catches missing bytes (3/4).
    fd = $fopen("work/test_fscanf.bin", "wb");
    in_32 = 32'b000x100z_001z000x_101xxxzz_100z111x;
    $fwrite(fd, "%u%c%c", in_32, in_32[15:8], in_32[7:0]);
    $fclose(fd);
    fd = $fopen("work/test_fscanf.bin", "rb");
    res = $fscanf(fd, "%u", out_64);
    if (res !== -1) begin
      $display("FAILED: $fscanf() #2c returned %d (%b)", res, out_64);
      passed = 1'b0;
    end
    $fclose(fd);

    // Check that a normal 32 bit %u suppression catches missing bytes.
    fd = $fopen("work/test_fscanf.bin", "wb");
    in_32 = 32'b000x100z_001z000x_101xxxzz_100z111x;
    $fwrite(fd, "%c%c", in_32[15:8], in_32[7:0]);
    $fclose(fd);
    fd = $fopen("work/test_fscanf.bin", "rb");
    res = $fscanf(fd, "%*u", out_32);
    if (res !== -1) begin
      $display("FAILED: $fscanf() #3 returned %d (%b)", res, out_32);
      passed = 1'b0;
    end
    $fclose(fd);

    // Check that a multiple read %u catches missing bytes (1/2).
    fd = $fopen("work/test_fscanf.bin", "wb");
    in_32 = 32'b000x100z_001z000x_101xxxzz_100z111x;
    ck_32 = 32'b00001000_00100000_10100000_10001110;
    $fwrite(fd, "%u%c%c", in_32, in_32[15:8], in_32[7:0]);
    $fclose(fd);
    fd = $fopen("work/test_fscanf.bin", "rb");
    res = $fscanf(fd, "%u%u", out_32, out_32x);
    if (res !== 1) begin
      $display("FAILED: $fscanf() #4 returned %d (%b)", res, out_32x);
      passed = 1'b0;
    end else begin
      if (ck_32 !== out_32) begin
        $display("FAILED: $fscanf() #4 %b !== %b", ck_32, out_32);
        passed = 1'b0;
      end
      if (out_32x !== 32'bx) begin
        $display("FAILED: $fscanf() #4 %b !== 32'bx", out_32x);
        passed = 1'b0;
      end
    end
    res = $fscanf(fd, "%u", out_32);
    if (res !== -1) begin
      $display("FAILED: $fscanf() #4 EOF returned %d (%b)", res, out_32);
      passed = 1'b0;
    end
    $fclose(fd);

    // Check that a suppression/read %u catches missing bytes (1/2).
    fd = $fopen("work/test_fscanf.bin", "wb");
    in_32 = 32'b000x100z_001z000x_101xxxzz_100z111x;
    $fwrite(fd, "%u%c%c", in_32, in_32[15:8], in_32[7:0]);
    $fclose(fd);
    fd = $fopen("work/test_fscanf.bin", "rb");
    out_32 = 32'bx;
    res = $fscanf(fd, "%*u%u", out_32);
    if (res !== -1) begin
      $display("FAILED: $fscanf() #5 returned %d (%b)", res, out_32);
      passed = 1'b0;
    end else if (out_32 !== 32'bx) begin
      $display("FAILED: $fscanf() #5 %b !== 32'bx", out_32);
      passed = 1'b0;
    end
    $fclose(fd);

    if (passed) $display("PASSED");
  end
endmodule
