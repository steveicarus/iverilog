// Check packed dimensions on typedef names in system function arguments.

package p;
  typedef logic [7:0] byte_t;
endpackage

typedef logic [3:0] byte_t;

module test;

  import p::*;
  reg failed;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %0d, got %0d", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;

    `check($bits(byte_t [0:1]), 16);
    `check($bits(byte_t [1:0]), 16);
    `check($bits(byte_t [1:0][2:0]), 48);
    `check($bits(p::byte_t [3:0]), 32);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
