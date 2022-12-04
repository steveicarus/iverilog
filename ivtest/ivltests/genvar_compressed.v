// Check that compressed assignment statements are supported for genvar loops.
// This is supported in SystemVerilog, but not in Verilog.

module test;

integer array1[0:3];
integer array2[1:4];
integer array3[1:8];
integer array4[1:16];
integer array5[10:10];
integer array6[1:8];
integer array7[1:16];
integer array8[1:16];
integer array9[0:3];
integer array10[4:7];
integer array11[0:3];

for (genvar i = 0; i < 4; i += 1) begin
  initial array1[i] = i;
end

for (genvar i = 4; i > 0; i -= 1) begin
  initial array2[i] = i;
end

for (genvar i = 1; i < 16; i *= 2) begin
  initial array3[i] = i;
end

for (genvar i = 16; i > 0; i /= 2) begin
  initial array4[i] = i;
end

for (genvar i = 10; i > 0; i %= 2) begin
  initial array5[i] = i;
end

for (genvar i = 1; i < 16; i <<= 1) begin
  initial array6[i] = i;
end

for (genvar i = 16; i > 0; i >>= 1) begin
  initial array7[i] = i;
end

for (genvar i = 16; i > 0; i >>>= 1) begin
  initial array8[i] = i;
end

for (genvar i = 0; i < 4; i |= i + 1) begin
  initial array9[i] = i;
end

for (genvar i = 7; i > 0; i &= i - 1) begin
  initial array10[i] = i;
end

for (genvar i = 0; i < 4; i ^= i + 1) begin
  initial array11[i] = i;
end

`define check(a) if (a[i] !== i) begin \
  failed = 1; \
  $display("FAILED(%d): Expected %0d, got %0d.", `__LINE__, i, a[i]); \
  end


integer i;
reg failed = 1'b0;

initial begin
  #1
  for (i = 0; i < 4; i = i + 1) begin
    `check(array1)
  end

  for (i = 4; i > 0; i = i - 1) begin
    `check(array2)
  end

  for (i = 1; i < 16; i = i * 2) begin
    `check(array3)
  end

  for (i = 16; i > 0; i = i / 2) begin
    `check(array4)
  end

  for (i = 10; i != 0; i = i % 2) begin
    `check(array5)
  end

  for (i = 1; i < 16; i = i << 2) begin
    `check(array6)
  end

  for (i = 16; i > 0; i = i >> 2) begin
    `check(array7)
  end

  for (i = 16; i > 0; i = i >>> 2) begin
    `check(array8)
  end

  for (i = 1; i < 4; i = i | (i + 1)) begin
    `check(array9)
  end

  for (i = 7; i > 0; i = i & (i - 1)) begin
    `check(array10)
  end

  for (i = 0; i < 4; i = i ^ (i + 1)) begin
    `check(array11)
  end

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
