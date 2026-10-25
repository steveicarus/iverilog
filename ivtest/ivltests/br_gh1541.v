// Check that X/Z specify path conditions enable the path delay.

`timescale 1ns/1ps

module M(input a, input c, output y);
  assign y = a;

  specify
    if (c) (a => y) = 3;
  endspecify
endmodule

module test;

  reg a, c;
  wire y;
  time start_time, change_time;
  reg failed;

  M i_m(a, c, y);

  always @(y) change_time = $time;

  `define check(val, exp) \
    c = val; \
    #10; \
    start_time = $time; \
    a = ~a; \
    #10; \
    if (y !== a || change_time - start_time !== exp) begin \
      $display("FAILED(%0d): condition %s expected delay %0d, got %0d", \
               `__LINE__, `"val`", exp, change_time - start_time); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    a = 1'b0;
    `check(1'b0, 0);
    `check(1'b1, 3);
    `check(1'bx, 3);
    `check(1'bz, 3);
    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
