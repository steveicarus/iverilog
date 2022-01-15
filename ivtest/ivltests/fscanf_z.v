module top;
  reg [15:0] out_16;
  reg [31:0] in_32, in_32x, out_32, out_32x;
  reg [63:0] out_64;
  integer res, fd;
  reg passed;

  initial begin
    passed = 1'b1;

    // Check that a normal 32 bit %z works as expected.
    fd = $fopen("work/test_fscanf.bin", "wb");
    in_32 = 32'b000x100z_001z000x_101xxxzz_100z111x;
    $fwrite(fd, "%z", in_32);
    $fclose(fd);
    fd = $fopen("work/test_fscanf.bin", "rb");
    res = $fscanf(fd, "%z", out_32);
    if (res !== 1) begin
      $display("FAILED: $fscanf() #1 returned %d", res);
      passed = 1'b0;
    end else if (in_32 !== out_32) begin
      $display("FAILED: #1 %b !== %b", in_32, out_32);
      passed = 1'b0;
    end
    res = $fscanf(fd, "%z", out_32);
    if (res !== -1) begin
      $display("FAILED: $fscanf() #1 EOF returned %d (%b)", res, out_32);
      passed = 1'b0;
    end
    $fclose(fd);

    // Check that a normal 32/64 bit %z works as expected. Do the write as
    // two 32 bit values to make sure the word order is correct.
    fd = $fopen("work/test_fscanf.bin", "wb");
    in_32 = 32'b000x100z_001z000x_101xxxzz_100z111x;
    $fwrite(fd, "%z", in_32);
    in_32x = 32'b0001000x_0010000x_0011000x_0100000x;
    $fwrite(fd, "%z", in_32x);
    $fclose(fd);
    fd = $fopen("work/test_fscanf.bin", "rb");
    res = $fscanf(fd, "%z", out_64);
    if (res !== 1) $display("FAILED: $fscanf() #2a returned %d", res);
    else if ({in_32x,in_32} !== out_64) begin
      $display("FAILED: #2a %b !== %b", {in_32x,in_32}, out_64);
      passed = 1'b0;
    end
    res = $fscanf(fd, "%z", out_32);
    if (res !== -1) begin
      $display("FAILED: $fscanf() #2a EOF returned %d (%b)", res, out_32);
      passed = 1'b0;
    end
    $fclose(fd);

    // Check that a normal 64/64 bit %z works as expected.
    fd = $fopen("work/test_fscanf.bin", "wb");
    in_32 = 32'b000x100z_001z000x_101xxxzz_100z111x;
    in_32x = 32'b0001000x_0010000x_0011000x_0100000x;
    $fwrite(fd, "%z", {in_32x,in_32});
    $fclose(fd);
    fd = $fopen("work/test_fscanf.bin", "rb");
    res = $fscanf(fd, "%z", out_64);
    if (res !== 1) $display("FAILED: $fscanf() #2b returned %d", res);
    else if ({in_32x,in_32} !== out_64) begin
      $display("FAILED: #2b %b !== %b", {in_32x,in_32}, out_64);
      passed = 1'b0;
    end
    res = $fscanf(fd, "%z", out_32);
    if (res !== -1) begin
      $display("FAILED: $fscanf() #2b EOF returned %d (%b)", res, out_32);
      passed = 1'b0;
    end
    $fclose(fd);

    // Check that a normal 64/32 bit %z works as expected.
    fd = $fopen("work/test_fscanf.bin", "wb");
    in_32 = 32'b000x100z_001z000x_101xxxzz_100z111x;
    in_32x = 32'b0001000x_0010000x_0011000x_0100000x;
    $fwrite(fd, "%z", {in_32x,in_32});
    $fclose(fd);
    fd = $fopen("work/test_fscanf.bin", "rb");
    res = $fscanf(fd, "%z%z", out_32,out_32x);
    if (res !== 2) $display("FAILED: $fscanf() #2c returned %d", res);
    else if ({in_32x,in_32} !== {out_32x, out_32}) begin
      $display("FAILED: #2c %b !== %b", {in_32x,in_32}, {out_32x,out_32});
      passed = 1'b0;
    end
    res = $fscanf(fd, "%z", out_32);
    if (res !== -1) begin
      $display("FAILED: $fscanf() #2c EOF returned %d (%b)", res, out_32);
      passed = 1'b0;
    end
    $fclose(fd);

    // Check that a 16 bit %z works as expected.
    fd = $fopen("work/test_fscanf.bin", "wb");
    in_32 = 32'b000x100z_001z000x_101xxxzz_100z111x;
    $fwrite(fd, "%z", in_32);
    $fclose(fd);
    fd = $fopen("work/test_fscanf.bin", "rb");
    res = $fscanf(fd, "%z", out_16);
    if (res !== 1) begin
      $display("FAILED: $fscanf() #3 returned %d", res);
      passed = 1'b0;
    end else if (in_32[15:0] !== out_16) begin
      $display("FAILED: #3 %b !== %b", in_32[15:0], out_16);
      passed = 1'b0;
    end
    res = $fscanf(fd, "%z", out_32);
    if (res !== -1) begin
      $display("FAILED: $fscanf() #3 EOF returned %d (%b)", res, out_32);
      passed = 1'b0;
    end
    $fclose(fd);

    // Check that a 16 bit %z works as expected even with a 32 bit variable.
    // All 32 bits are read but we truncate and zero fill the result.
    fd = $fopen("work/test_fscanf.bin", "wb");
    in_32 = 32'b000x100z_001z000x_101xxxzz_100z111x;
    $fwrite(fd, "%z", in_32);
    $fclose(fd);
    fd = $fopen("work/test_fscanf.bin", "rb");
    res = $fscanf(fd, "%16z", out_32);
    if (res !== 1) begin
      $display("FAILED: $fscanf() #4 returned %d", res);
      passed = 1'b0;
    end else if (in_32[15:0] !== out_32) begin
      $display("FAILED: #4 %b !== %b", in_32[15:0], out_32);
      passed = 1'b0;
    end
    res = $fscanf(fd, "%z", out_32);
    if (res !== -1) begin
      $display("FAILED: $fscanf() #4 EOF returned %d (%b)", res, out_32);
      passed = 1'b0;
    end
    $fclose(fd);

    // Check that a 32 bit %z works with a 64 bit variable when sized.
    fd = $fopen("work/test_fscanf.bin", "wb");
    in_32 = 32'b000x100z_001z000x_101xxxzz_100z111x;
    $fwrite(fd, "%z", in_32);
    $fclose(fd);
    fd = $fopen("work/test_fscanf.bin", "rb");
    res = $fscanf(fd, "%32z", out_64);
    if (res !== 1) begin
      $display("FAILED: $fscanf() #5 returned %d", res);
      passed = 1'b0;
    end else if (in_32 !== out_64) begin
      $display("FAILED: #5 %b !== %b", in_32, out_64);
      passed = 1'b0;
    end
    res = $fscanf(fd, "%z", out_32);
    if (res !== -1) begin
      $display("FAILED: $fscanf() #5 EOF returned %d (%b)", res, out_32);
      passed = 1'b0;
    end
    $fclose(fd);

    // Check that by default one element is suppressed.
    fd = $fopen("work/test_fscanf.bin", "wb");
    in_32x = 32'b0001000x_0010000x_0011000x_0100000x;
    $fwrite(fd, "%z", in_32x);
    in_32 = 32'b000x100z_001z000x_101xxxzz_100z111x;
    $fwrite(fd, "%z", in_32);
    $fclose(fd);
    fd = $fopen("work/test_fscanf.bin", "rb");
    res = $fscanf(fd, "%*z%z", out_32);
    if (res !== 1) begin
      $display("FAILED: $fscanf() #6 returned %d", res);
      passed = 1'b0;
    end else if (in_32 !== out_32) begin
      $display("FAILED: #6 %b !== %b", in_32, out_32);
      passed = 1'b0;
    end
    res = $fscanf(fd, "%z", out_32);
    if (res !== -1) begin
      $display("FAILED: $fscanf() #6 EOF returned %d (%b)", res, out_32);
      passed = 1'b0;
    end
    $fclose(fd);

    // Check that multiple elements can be suppressed (exact count).
    fd = $fopen("work/test_fscanf.bin", "wb");
    in_32x = 32'b0001000x_0010000x_0011000x_0100000x;
    $fwrite(fd, "%z%z", in_32x, in_32x);
    in_32 = 32'b000x100z_001z000x_101xxxzz_100z111x;
    $fwrite(fd, "%z", in_32);
    $fclose(fd);
    fd = $fopen("work/test_fscanf.bin", "rb");
    res = $fscanf(fd, "%*64z%z", out_32);
    if (res !== 1) begin
      $display("FAILED: $fscanf() #7 returned %d", res);
      passed = 1'b0;
    end else if (in_32 !== out_32) begin
      $display("FAILED: #7 %b !== %b", in_32, out_32);
      passed = 1'b0;
    end
    res = $fscanf(fd, "%z", out_32);
    if (res !== -1) begin
      $display("FAILED: $fscanf() #7 EOF returned %d (%b)", res, out_32);
      passed = 1'b0;
    end
    $fclose(fd);

    // Check that multiple elements can be suppressed (minimum count).
    fd = $fopen("work/test_fscanf.bin", "wb");
    in_32x = 32'b0001000x_0010000x_0011000x_0100000x;
    $fwrite(fd, "%z%z", in_32x, in_32x);
    in_32 = 32'b000x100z_001z000x_101xxxzz_100z111x;
    $fwrite(fd, "%z", in_32);
    $fclose(fd);
    fd = $fopen("work/test_fscanf.bin", "rb");
    res = $fscanf(fd, "%*33z%z", out_32);
    if (res !== 1) begin
      $display("FAILED: $fscanf() #8 returned %d", res);
      passed = 1'b0;
    end else if (in_32 !== out_32) begin
      $display("FAILED: #8 %b !== %b", in_32, out_32);
      passed = 1'b0;
    end
    res = $fscanf(fd, "%z", out_32);
    if (res !== -1) begin
      $display("FAILED: $fscanf() #8 EOF returned %d (%b)", res, out_32);
      passed = 1'b0;
    end
    $fclose(fd);

    if (passed) $display("PASSED");
  end
endmodule
