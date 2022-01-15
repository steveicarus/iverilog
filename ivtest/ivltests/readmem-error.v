module top;
  reg [24*8-1:0] str;
  real rval;
  reg [7:0] array [0:7];
  reg [7:0] array2 [8:15];
  reg [7:0] array3 [-1:7];
  integer idx, istr;

  task clear_array;
    for (idx = 0; idx < 8; idx = idx + 1) begin
      array[idx] = 0;
      array2[idx+8] = 0;
    end
  endtask

  initial begin
    // An invalid string.
    $readmemb(str, array);
    $readmemb(istr, array);

    // Check a valid string.
    str = "ivltests/readmemb.txt";
    $readmemb(str, array);
    for (idx = 0; idx < 8; idx = idx + 1) begin
      if (array[idx] !== idx + 1) begin
        $display("Failed: for index %0d of readmemb 1, expected %0d, got %0d",
                 idx, idx+1, array[idx]);
      end
    end

    // Check a string with a non-printing character.
    str[7:0] = 'd2;
    $readmemb(str, array);

    // This should load, but will print a warning about the real.
    rval = 0.0;
    clear_array;
    $readmemb("ivltests/readmemb.txt", array, rval);
    for (idx = 0; idx < 8; idx = idx + 1) begin
      if (array[idx] !== idx + 1) begin
        $display("Failed: for index %0d of readmemb 2, expected %0d, got %0d",
                 idx, idx+1, array[idx]);
      end
    end

    // This should load, but will print a warning about the real.
    rval = 7.0;
    clear_array;
    $readmemb("ivltests/readmemb.txt", array, 0, rval);
    for (idx = 0; idx < 8; idx = idx + 1) begin
      if (array[idx] !== idx + 1) begin
        $display("Failed: for index %0d of readmemb 3, expected %0d, got %0d",
                 idx, idx+1, array[idx]);
      end
    end

    // These should not load the array.
    clear_array;
    $readmemb("ivltests/readmemb.txt", array, -1, 7);
    for (idx = 0; idx < 8; idx = idx + 1) begin
      if (array[idx] !== 0) begin
        $display("Failed: for index %0d of readmemb 4, expected 0, got %0d",
                 idx, array[idx]);
      end
    end

    $readmemb("ivltests/readmemb.txt", array2, 7, 15);
    for (idx = 8; idx < 16; idx = idx + 1) begin
      if (array2[idx] !== 0) begin
        $display("Failed: for index %0d of readmemb 5, expected 0, got %0d",
                 idx, array2[idx]);
      end
    end

    $readmemb("ivltests/readmemb.txt", array, 0, 8);
    for (idx = 0; idx < 8; idx = idx + 1) begin
      if (array[idx] !== 0) begin
        $display("Failed: for index %0d of readmemb 6, expected 0, got %0d",
                 idx, array[idx]);
      end
    end

    $readmemb("ivltests/readmemb.txt", array2, 8, 16);
    for (idx = 8; idx < 16; idx = idx + 1) begin
      if (array2[idx] !== 0) begin
        $display("Failed: for index %0d of readmemb 7, expected 0, got %0d",
                 idx, array2[idx]);
      end
    end

    // Check that a warning is printed if we have the wrong number of values.
    clear_array;
    $readmemb("ivltests/readmemb.txt", array, 0, 6);
    for (idx = 0; idx < 7; idx = idx + 1) begin
      if (array[idx] !== idx + 1) begin
        $display("Failed: for index %0d of readmemb 8, expected %0d, got %0d",
                 idx, idx+1, array[idx]);
      end
    end
    if (array[7] !== 0) begin
        $display("Failed: for index 7 of readmemb 8, expected 0, got %0d",
                 array[7]);
    end

    $readmemb("ivltests/readmemb.txt", array3, -1, 7);
    for (idx = -1; idx < 7; idx = idx + 1) begin
      if ($signed(array3[idx]) !== idx + 2) begin
        $display("Failed: for index %0d of readmemb 9, expected %0d, got %0d",
                 idx, idx+2, array3[idx]);
      end
    end
    if (array3[7] !== 8'bx) begin
        $display("Failed: for index 7 of readmemb 9, expected 'dx, got %0d",
                 array3[7]);
    end

    // Check what an invalid token returns.
    $readmemb("ivltests/readmem-error.txt", array);


    // An invalid string.
    str = 'bx;
    $readmemh(str, array);
    $readmemh(istr, array);

    // Check a valid string.
    str = "ivltests/readmemh.txt";
    $readmemh(str, array);
    for (idx = 0; idx < 8; idx = idx + 1) begin
      if (array[idx] !== idx + 1) begin
        $display("Failed: for index %0d of readmemh 1, expected %0d, got %0d",
                 idx, idx+1, array[idx]);
      end
    end

    // Check a string with a non-printing character.
    str[7:0] = 'd2;
    $readmemh(str, array);

    // This should load, but will print a warning about the real.
    rval = 0.0;
    clear_array;
    $readmemh("ivltests/readmemh.txt", array, rval);
    for (idx = 0; idx < 8; idx = idx + 1) begin
      if (array[idx] !== idx + 1) begin
        $display("Failed: for index %0d of readmemh 2, expected %0d, got %0d",
                 idx, idx+1, array[idx]);
      end
    end

    // This should load, but will print a warning about the real.
    rval = 7.0;
    clear_array;
    $readmemh("ivltests/readmemh.txt", array, 0, rval);
    for (idx = 0; idx < 8; idx = idx + 1) begin
      if (array[idx] !== idx + 1) begin
        $display("Failed: for index %0d of readmemh 3, expected %0d, got %0d",
                 idx, idx+1, array[idx]);
      end
    end

    // These should not load the array.
    clear_array;
    $readmemh("ivltests/readmemh.txt", array, -1, 7);
    for (idx = 0; idx < 8; idx = idx + 1) begin
      if (array[idx] !== 0) begin
        $display("Failed: for index %0d of readmemh 4, expected 0, got %0d",
                 idx, array[idx]);
      end
    end

    $readmemh("ivltests/readmemh.txt", array2, 7, 15);
    for (idx = 8; idx < 16; idx = idx + 1) begin
      if (array2[idx] !== 0) begin
        $display("Failed: for index %0d of readmemh 5, expected 0, got %0d",
                 idx, array2[idx]);
      end
    end

    $readmemh("ivltests/readmemh.txt", array, 0, 8);
    for (idx = 0; idx < 8; idx = idx + 1) begin
      if (array[idx] !== 0) begin
        $display("Failed: for index %0d of readmemh 6, expected 0, got %0d",
                 idx, array[idx]);
      end
    end

    $readmemh("ivltests/readmemh.txt", array2, 8, 16);
    for (idx = 8; idx < 16; idx = idx + 1) begin
      if (array2[idx] !== 0) begin
        $display("Failed: for index %0d of readmemh 7, expected 0, got %0d",
                 idx, array2[idx]);
      end
    end

    // Check that a warning is printed if we have the wrong number of values.
    clear_array;
    $readmemh("ivltests/readmemh.txt", array, 0, 6);
    for (idx = 0; idx < 7; idx = idx + 1) begin
      if (array[idx] !== idx + 1) begin
        $display("Failed: for index %0d of readmemh 8, expected %0d, got %0d",
                 idx, idx+1, array[idx]);
      end
    end
    if (array[7] !== 0) begin
        $display("Failed: for index 7 of readmemh 8, expected 0, got %0d",
                 array[7]);
    end

    $readmemh("ivltests/readmemh.txt", array3, -1, 7);
    for (idx = -1; idx < 7; idx = idx + 1) begin
      if ($signed(array3[idx]) !== idx + 2) begin
        $display("Failed: for index %0d of readmemh 9, expected %0d, got %0d",
                 idx, idx+2, array3[idx]);
      end
    end
    if (array3[7] !== 8'bx) begin
        $display("Failed: for index 7 of readmemh 9, expected 'dx, got %0d",
                 array3[7]);
    end

    // Check what an invalid token returns.
    $readmemh("ivltests/readmem-error.txt", array);

  end
endmodule
