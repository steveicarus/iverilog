`begin_keywords "1364-2005"
module top;
  localparam string = "ab";
  localparam rg_res = string & 9'h1ff;

  reg passed;
  integer fd, res;
  reg [8:0] rg;
  reg [7:0] mem [31:0];

  initial begin
    passed = 1'b1;

    fd = $fopen("ThisFileDoesNotExist.txt", "r");

    res = $fread(rg, fd); // Try to read from an invalid fd.
    if (res != 0) begin
      $display("$fread (register fd) count is wrong, expected 0, got %0d", res);
      passed = 1'b0;
    end
    if (rg !== 9'bx) begin
      $display("$fread (register fd) value is wrong, expected 9'bx, got %b",
               rg);
      passed = 1'b0;
    end

    fd = $fopen("ivltests/fread.txt", "r");

    res = $fread(mem, fd, -1); // Try an invalid start
    if (res != 0) begin
      $display("$fread (mem. start) count is wrong, expected 0, got %0d", res);
      passed = 1'b0;
    end
    if (mem[0] !== 8'bx) begin
      $display("$fread (mem. start[0]) value is wrong, expected 8'bx, got %b",
               mem[0]);
      passed = 1'b0;
    end
    if (mem[15] !== 8'bx) begin
      $display("$fread (mem. start[15]) value is wrong, expected 8'bx, got %b",
               mem[15]);
      passed = 1'b0;
    end
    if (mem[31] !== 8'bx) begin
      $display("$fread (mem. start[31]) value is wrong, expected 8'bx, got %b",
               mem[31]);
      passed = 1'b0;
    end

    // Check $fread with a register value.
    res = $fread(rg, fd); // Load with the lower nine bits of "ab".
    if (res != 2) begin
      $display("$fread (register) count is wrong, expected 2, got %0d", res);
      passed = 1'b0;
    end
    if (rg !== rg_res) begin
      $display("$fread (register) value is wrong, expected %b, got %b",
               rg_res, rg);
      passed = 1'b0;
    end

    // Check $fread with a memory.
    res = $fread(mem, fd, 0, 2); // Load 0 with "0" and 1 with "1".
    if (res != 2) begin
      $display("$fread (mem. 1) count is wrong, expected 2, got %0d", res);
      passed = 1'b0;
    end
    if (mem[0] !== "0") begin
      $display("$fread (mem. 1[0]) value is wrong, expected %b, got %b",
               "0", mem[0]);
      passed = 1'b0;
    end
    if (mem[1] !== "1") begin
      $display("$fread (mem. 1[1]) value is wrong, expected %b, got %b",
               "1", mem[1]);
      passed = 1'b0;
    end

    res = $fread(mem, fd, 31); // Load 31 with "z".
    if (res != 1) begin
      $display("$fread (mem. 2) count is wrong, expected 1, got %0d", res);
      passed = 1'b0;
    end
    if (mem[31] !== "z") begin
      $display("$fread (mem. 2[31]) value is wrong, expected %b, got %b",
               "z", mem[31]);
      passed = 1'b0;
    end

    res = $fread(mem, fd, 31, 2); // Load 31 with "y" and warns.
    if (res != 1) begin
      $display("$fread (mem. 3) count is wrong, expected 1, got %0d", res);
      passed = 1'b0;
    end
    if (mem[31] !== "y") begin
      $display("$fread (mem. 3[31]) value is wrong, expected %b, got %b",
               "y", mem[31]);
      passed = 1'b0;
    end

    res = $fread(mem, fd); // Load with  repeated "0" .. "9" pattern.
    if (res != 32) begin
      $display("$fread (mem. 4) count is wrong, expected 32, got %0d", res);
      passed = 1'b0;
    end
    // Just check the end values and a value in the middle (15).
    if (mem[0] !== "0") begin
      $display("$fread (mem. 4[0]) value is wrong, expected %b, got %b",
               "0", mem[0]);
      passed = 1'b0;
    end
    if (mem[15] !== "5") begin
      $display("$fread (mem. 4[15]) value is wrong, expected %b, got %b",
               "5", mem[15]);
      passed = 1'b0;
    end
    if (mem[31] !== "1") begin
      $display("$fread (mem. 4[31]) value is wrong, expected %b, got %b",
               "1", mem[31]);
      passed = 1'b0;
    end

    // This only gets the trailing new line.
    rg = 9'bx;
    res = $fread(rg, fd);
    if (res != 1) begin
      $display("$fread (EOL) count is wrong, expected 1, got %0d", res);
      passed = 1'b0;
    end
    if (rg !== 9'h0xx) begin
      $display("$fread (EOL value is wrong, expected 9'b0xx, got %b", rg);
      passed = 1'b0;
    end

   // There are no bits left so this array should be the same.
    res = $fread(mem, fd);
    if (res != 0) begin
      $display("$fread (mem. EOL) count is wrong, expected 0, got %0d", res);
      passed = 1'b0;
    end
    // Just check the end values and a value in the middle (15).
    if (mem[0] !== "0") begin
      $display("$fread (mem. EOL[0]) value is wrong, expected %b, got %b",
               "0", mem[0]);
      passed = 1'b0;
    end
    if (mem[15] !== "5") begin
      $display("$fread (mem. EOL[15]) value is wrong, expected %b, got %b",
               "5", mem[15]);
      passed = 1'b0;
    end
    if (mem[31] !== "1") begin
      $display("$fread (mem. EOL[31]) value is wrong, expected %b, got %b",
               "1", mem[31]);
      passed = 1'b0;
    end

    $fclose(fd);

    if (passed) $display("PASSED");
    else $display("FAILED");
  end
endmodule
`end_keywords
