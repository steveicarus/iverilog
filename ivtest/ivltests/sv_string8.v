// Check that null-bytes are removed when converting to string type.

module test;

  reg failed = 1'b0;

  `define check(val, exp) \
    if (val != exp) begin \
      $display("FAILED(%0d): Expected '%0s', got '%0s'.", `__LINE__, exp, val); \
      failed = 1'b1; \
    end

  string s;
  reg [47:0] x;

  initial begin
    s = "\000a\000b\000";
    `check(s, "ab");

    s = string'("\000a\000b\000");
    `check(s, "ab");

    s = string'(48'h0061006200);
    `check(s, "ab");

    x = 48'h0061006200;
    s = string'(x);
    `check(s, "ab");

    s = "cd";
    s = {s, "\000a\000b\000"};
    `check(s, "cdab");

    s = "cd";
    s = {"\000a\000b\000", s};
    `check(s, "abcd");

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
