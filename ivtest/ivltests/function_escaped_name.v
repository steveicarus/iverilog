// Check that an escaped function name matching a hierarchical function name
// calls the correct function in procedural and continuous calls.

module M;
  function [7:0] f;
    input [7:0] value;
    f = value + 2;
  endfunction
endmodule

module test;
  M g();

  function [7:0] \g.f ;
    input [7:0] value;
    \g.f = value + 1;
  endfunction

  reg [7:0] value;
  wire [7:0] escaped = \g.f (value);
  wire [7:0] nested = g.f(value);
  reg failed;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %0d, got %0d", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    value = 5;
    #1;
    `check(\g.f (value), 6);
    `check(g.f(value), 7);
    `check(escaped, 6);
    `check(nested, 7);

    if (!failed) $display("PASSED");
  end
endmodule
