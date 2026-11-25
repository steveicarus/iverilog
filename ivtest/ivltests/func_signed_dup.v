// Check that a signed function result keeps its width and signedness where the
// compiler copies the expression that holds the call: a case selector it narrows
// to the case items, the branch a constant ?: condition picks, and x in (x + 1) + 2.

module test;

  reg failed;
  reg signed [2:0] a, b;
  reg y_case, y_if;
  wire y_tern;
  wire [3:0] y_add;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %h, got %h", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  function automatic signed [2:0] f(input signed [2:0] u);
    f = u;
  endfunction

  always @* case (a < f(b)) 4'd1: y_case = 1'b1; default: y_case = 1'b0; endcase
  assign y_tern = 1 ? (a < f(b)) : 1'b0;
  assign y_add = ((a < f(b)) + 4'd1) + 4'd2;
  always @* if (1 ? f(b) : a) y_if = 1'b1; else y_if = 1'b0;

  initial begin
    failed = 1'b0;
    a = -3'sd1;
    b = 3'sd1;
    #1;
    `check(y_case, 1'b1);
    `check(y_tern, 1'b1);
    `check(y_add, 4'd4);
    `check(y_if, 1'b1);
    b = 3'sd0;
    #1;
    `check(y_if, 1'b0);
    a = 3'sd1;
    b = -3'sd1;
    #1;
    `check(y_case, 1'b0);
    `check(y_tern, 1'b0);
    `check(y_add, 4'd3);
    if (!failed)
      $display("PASSED");
  end

endmodule
