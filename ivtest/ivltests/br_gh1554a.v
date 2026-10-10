// Check that an empty named port connection suppresses the input default.

module M(input wire [3:0] p = 4'h7, output wire [3:0] y);
  assign y = p;
endmodule

module test;
  wire [3:0] omitted, empty, positional, connected;
  bit failed = 1'b0;

  M i_omitted(.y(omitted));
  M i_empty(.p(), .y(empty));
  M i_positional(, positional);
  M i_connected(.p(4'h3), .y(connected));

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %h, got %h", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    #1;
    `check(omitted, 4'h7);
    `check(empty, 4'bzzzz);
    `check(positional, 4'h7);
    `check(connected, 4'h3);

    if (!failed) begin
      $display("PASSED");
    end
  end
endmodule
