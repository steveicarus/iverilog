module top;
  reg [20*8-1:0] str;
  real rval;
  reg [7:0] array [0:7];
  reg [7:0] array2 [8:15];
  reg [7:0] check [0:7];
  integer idx, istr;


  initial begin
    for (idx = 0; idx < 8; idx = idx + 1) array[idx] = idx + 1;
    for (idx = 8; idx < 16; idx = idx + 1) array2[idx] = 0;

    // An invalid string.
    $writememb(str, array);
    $writememb(istr, array);

    // Check a valid string.
    str = "work/writemem.txt";
    for (idx = 0; idx < 8; idx = idx + 1) check[idx] = 0;
    $writememb(str, array);
    $readmemb(str, check);
    for (idx = 0; idx < 8; idx = idx + 1) begin
      if (check[idx] !== idx + 1) begin
        $display("Failed: for index %0d of writememb 1, expected %0d, got %0d",
                 idx, idx+1, check[idx]);
      end
    end

    // Check a string with a non-printing character.
    str[7:0] = 'd2;
    $writememb(str, array);

    // This should write, but will print a warning about the real.
    rval = 0.0;
    for (idx = 0; idx < 8; idx = idx + 1) check[idx] = 0;
    $writememb("work/writemem.txt", array, rval);
    $readmemb("work/writemem.txt", check);
    for (idx = 0; idx < 8; idx = idx + 1) begin
      if (check[idx] !== idx + 1) begin
        $display("Failed: for index %0d of writememb 2, expected %0d, got %0d",
                 idx, idx+1, check[idx]);
      end
    end

    // This should write, but will print a warning about the real.
    rval = 7.0;
    for (idx = 0; idx < 8; idx = idx + 1) check[idx] = 0;
    $writememb("work/writemem.txt", array, 0, rval);
    $readmemb("work/writemem.txt", check);
    for (idx = 0; idx < 8; idx = idx + 1) begin
      if (check[idx] !== idx + 1) begin
        $display("Failed: for index %0d of writememb 3, expected %0d, got %0d",
                 idx, idx+1, check[idx]);
      end
    end

    // These should not write the array.
    for (idx = 0; idx < 8; idx = idx + 1) check[idx] = 0;
    $writememb("work/writemem.txt", check, -1, 7);
    $readmemb("work/writemem.txt", check);
    for (idx = 0; idx < 8; idx = idx + 1) begin
      if (check[idx] !== idx + 1) begin
        $display("Failed: for index %0d of writememb 4, expected %0d, got %0d",
                 idx, idx+1, check[idx]);
      end
    end

    for (idx = 0; idx < 8; idx = idx + 1) check[idx] = 0;
    $writememb("work/writemem.txt", array2, 7, 15);
    $readmemb("work/writemem.txt", check);
    for (idx = 0; idx < 8; idx = idx + 1) begin
      if (check[idx] !== idx + 1) begin
        $display("Failed: for index %0d of writememb 5, expected %0d, got %0d",
                 idx, idx+1, check[idx]);
      end
    end

    for (idx = 0; idx < 8; idx = idx + 1) check[idx] = 0;
    $writememb("work/writemem.txt", check, 0, 8);
    $readmemb("work/writemem.txt", check);
    for (idx = 0; idx < 8; idx = idx + 1) begin
      if (check[idx] !== idx + 1) begin
        $display("Failed: for index %0d of writememb 6, expected %0d, got %0d",
                 idx, idx+1, check[idx]);
      end
    end

    for (idx = 0; idx < 8; idx = idx + 1) check[idx] = 0;
    $writememb("work/writemem.txt", array2, 8, 16);
    $readmemb("work/writemem.txt", check);
    for (idx = 0; idx < 8; idx = idx + 1) begin
      if (check[idx] !== idx + 1) begin
        $display("Failed: for index %0d of writememb 7, expected %0d, got %0d",
                 idx, idx+1, check[idx]);
      end
    end

    // Check that we can write part of an array.
    for (idx = 0; idx < 8; idx = idx + 1) check[idx] = 0;
    $writememb("work/writemem.txt", array, 0, 6);
    $readmemb("work/writemem.txt", check, 0, 6);
    for (idx = 0; idx < 7; idx = idx + 1) begin
      if (check[idx] !== idx + 1) begin
        $display("Failed: for index %0d of writememb 8, expected %0d, got %0d",
                 idx, idx+1, check[idx]);
      end
    end
    if (check[7] !== 0) begin
        $display("Failed: for index 7 of writememb 8, expected 0, got %0d",
                 check[7]);
    end


    // An invalid string.
    str = 'bx;
    $writememh(str, array);
    $writememh(istr, array);

    // Check a valid string.
    str = "work/writemem.txt";
    for (idx = 0; idx < 8; idx = idx + 1) check[idx] = 0;
    $writememh(str, array);
    $readmemh(str, check);
    for (idx = 0; idx < 8; idx = idx + 1) begin
      if (check[idx] !== idx + 1) begin
        $display("Failed: for index %0d of writememh 1, expected %0d, got %0d",
                 idx, idx+1, check[idx]);
      end
    end

    // Check a string with a non-printing character.
    str[7:0] = 'd2;
    $writememh(str, array);

    // This should write, but will print a warning about the real.
    rval = 0.0;
    for (idx = 0; idx < 8; idx = idx + 1) check[idx] = 0;
    $writememh("work/writemem.txt", array, rval);
    $readmemh("work/writemem.txt", check);
    for (idx = 0; idx < 8; idx = idx + 1) begin
      if (check[idx] !== idx + 1) begin
        $display("Failed: for index %0d of writememh 2, expected %0d, got %0d",
                 idx, idx+1, check[idx]);
      end
    end

    // This should write, but will print a warning about the real.
    rval = 7.0;
    for (idx = 0; idx < 8; idx = idx + 1) check[idx] = 0;
    $writememh("work/writemem.txt", array, 0, rval);
    $readmemh("work/writemem.txt", check);
    for (idx = 0; idx < 8; idx = idx + 1) begin
      if (check[idx] !== idx + 1) begin
        $display("Failed: for index %0d of writememh 3, expected %0d, got %0d",
                 idx, idx+1, check[idx]);
      end
    end

    // These should not write the array.
    for (idx = 0; idx < 8; idx = idx + 1) check[idx] = 0;
    $writememh("work/writemem.txt", check, -1, 7);
    $readmemh("work/writemem.txt", check);
    for (idx = 0; idx < 8; idx = idx + 1) begin
      if (check[idx] !== idx + 1) begin
        $display("Failed: for index %0d of writememh 4, expected %0d, got %0d",
                 idx, idx+1, check[idx]);
      end
    end

    for (idx = 0; idx < 8; idx = idx + 1) check[idx] = 0;
    $writememh("work/writemem.txt", array2, 7, 15);
    $readmemh("work/writemem.txt", check);
    for (idx = 0; idx < 8; idx = idx + 1) begin
      if (check[idx] !== idx + 1) begin
        $display("Failed: for index %0d of writememh 5, expected %0d, got %0d",
                 idx, idx+1, check[idx]);
      end
    end

    for (idx = 0; idx < 8; idx = idx + 1) check[idx] = 0;
    $writememh("work/writemem.txt", check, 0, 8);
    $readmemh("work/writemem.txt", check);
    for (idx = 0; idx < 8; idx = idx + 1) begin
      if (check[idx] !== idx + 1) begin
        $display("Failed: for index %0d of writememh 6, expected %0d, got %0d",
                 idx, idx+1, check[idx]);
      end
    end

    for (idx = 0; idx < 8; idx = idx + 1) check[idx] = 0;
    $writememh("work/writemem.txt", array2, 8, 16);
    $readmemh("work/writemem.txt", check);
    for (idx = 0; idx < 8; idx = idx + 1) begin
      if (check[idx] !== idx + 1) begin
        $display("Failed: for index %0d of writememh 7, expected %0d, got %0d",
                 idx, idx+1, check[idx]);
      end
    end

    // Check that we can write part of an array.
    for (idx = 0; idx < 8; idx = idx + 1) check[idx] = 0;
    $writememh("work/writemem.txt", array, 0, 6);
    $readmemh("work/writemem.txt", check, 0, 6);
    for (idx = 0; idx < 7; idx = idx + 1) begin
      if (check[idx] !== idx + 1) begin
        $display("Failed: for index %0d of writememh 8, expected %0d, got %0d",
                 idx, idx+1, check[idx]);
      end
    end
    if (check[7] !== 0) begin
        $display("Failed: for index 7 of writememh 8, expected 0, got %0d",
                 check[7]);
    end

  end
endmodule
