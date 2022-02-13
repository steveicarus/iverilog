// Check that passing a data type to $bits works as expected

module test;

bit failed = 1'b0;

`define check(type, value) \
  if ($bits(type) !== value) begin \
    $display("FAILED: $bits(", `"type`", ") is %0d", $bits(type), " expected %0d", value); \
    failed = 1'b1; \
  end

typedef int T1;
typedef int T2[3:0];

initial begin
  `check(reg, 1);
  `check(logic, 1);
  `check(bit, 1);
  `check(logic [3:0], 4);
  `check(byte, 8);
  `check(shortint, 16);
  `check(int, 32);
  `check(longint, 64);
  `check(struct packed { int x; shortint y; }, 32 + 16);
  `check(T1, 32);
  `check(T2, 4 * 32);

  if (failed == 1'b0) begin
    $display("PASSED");
  end
end

endmodule
