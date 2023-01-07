// Check that null-bytes are ignored when assigning to an element of a string
// type variable.

module test;

  reg failed = 1'b0;

  `define check(val, exp) \
    if (val != exp) begin \
      $display("FAILED(%0d): Expected '%0s', got '%0s'.", `__LINE__, exp, val); \
      failed = 1'b1; \
    end

  string s;
  byte x;

  initial begin
    s = "Test";

    s[1] = 8'h00; // This should be ignored
    `check(s, "Test");
    s[1] = "\000"; // This should be ignored
    `check(s, "Test");
    x = 8'h00;
    s[1] = x; // This should be ignored
    `check(s, "Test");

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
