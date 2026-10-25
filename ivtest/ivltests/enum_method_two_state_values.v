// Check that enum methods work on two-state enums wider than 32 bits.

module test;
  enum bit [31:0] {A32 = 32'h80000001, B32 = 1, C32 = 32'hffffffff} e32;
  enum bit [32:0] {A33 = 33'h100000001, B33 = 1, C33 = 33'h100000002} e33;
  enum bit [63:0] {
    A64 = 64'h8000000000000001, B64 = 1, C64 = 64'h100000001
  } e64;
  enum longint {SA = -64'sh100000001, SB = -1, SC = 64'sh100000001} es64;
  bit failed = 1'b0;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  `define check_name(val, exp) \
    if (val != exp) begin \
      $display("FAILED(%0d). '%s' expected %s, got %s", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  `define check_methods(value, first, middle, last) \
    value = first; \
    `check_name(value.name(), `"first`"); \
    `check(value.next(), middle); \
    `check(value.prev(), last); \
    `check(value.next(0), first); \
    `check(value.prev(0), first); \
    `check(value.next(2), last); \
    `check(value.prev(2), middle); \
    value = middle; \
    `check_name(value.name(), `"middle`"); \
    `check(value.next(), last); \
    `check(value.prev(), first); \
    value = last; \
    `check_name(value.name(), `"last`"); \
    `check(value.next(), first); \
    `check(value.prev(), middle);

  initial begin
    `check_methods(e32, A32, B32, C32);
    `check_methods(e33, A33, B33, C33);
    `check_methods(e64, A64, B64, C64);
    `check_methods(es64, SA, SB, SC);

    if (!failed) $display("PASSED");
  end
endmodule
