module shift_immediate;
  reg failed = 1'b0;
  reg [7:0] value;
  reg signed [7:0] signed_value;
  reg [129:0] wide_value;

  task check8;
    input [7:0] actual;
    input [7:0] expected;
    input [8*32-1:0] name;
    begin
      if (actual !== expected) begin
        $display("FAILED %0s: got %b, expected %b", name, actual, expected);
        failed = 1'b1;
      end
    end
  endtask

  task check130;
    input [129:0] actual;
    input [129:0] expected;
    input [8*32-1:0] name;
    begin
      if (actual !== expected) begin
        $display("FAILED %0s: got %b, expected %b", name, actual, expected);
        failed = 1'b1;
      end
    end
  endtask

  initial begin
    value = 8'b10110011;
    signed_value = 8'b10110011;
    wide_value = 130'h2_0123456789abcdef_fedcba9876543210;

    check8(value << 0,  8'b10110011, "left zero");
    check8(value << 3,  8'b10011000, "left narrow");
    check8(value <<< 3, 8'b10011000, "arithmetic left");
    check8(value << 8,  8'b00000000, "left width");
    check8(value << 12, 8'b00000000, "left over width");

    check8(value >> 0,  8'b10110011, "right zero");
    check8(value >> 3,  8'b00010110, "right narrow");
    check8(value >>> 3, 8'b00010110, "unsigned arithmetic right");
    check8(value >> 8,  8'b00000000, "right width");
    check8(value >> 12, 8'b00000000, "right over width");

    check8(signed_value >>> 0,  8'b10110011, "signed zero");
    check8(signed_value >>> 3,  8'b11110110, "signed narrow");
    check8(signed_value >>> 8,  8'b11111111, "signed width");
    check8(signed_value >>> 12, 8'b11111111, "signed over width");

    check130(wide_value << 65, {wide_value[64:0], 65'b0}, "wide left");
    check130(wide_value >> 65, {65'b0, wide_value[129:65]}, "wide right");
    check130($signed(wide_value) >>> 65,
             {{65{wide_value[129]}}, wide_value[129:65]}, "wide signed");

    check8(value << -1, 8'b00000000, "negative distance");
    check8(value >> 33'h100000000, 8'b00000000, "large distance");
    check8(value << 1'bx, 8'bxxxxxxxx, "unknown distance");

    if (!failed)
      $display("PASSED");
  end
endmodule
