// Check that an empty named port suppresses its default with .* in either order.

module M(input wire [3:0] p = 4'h7, output wire [3:0] y);
  assign y = p;
endmodule

module test;
  wire [3:0] p = 4'h3;
  wire [3:0] empty_before, empty_after, connected;
  bit failed = 1'b0;

  M i_before(.p(), .y(empty_before), .*);
  M i_after(.*, .p(), .y(empty_after));
  M i_connected(.y(connected), .*);

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %h, got %h", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    #1;
    `check(empty_before, 4'bzzzz);
    `check(empty_after, 4'bzzzz);
    `check(connected, 4'h3);

    if (!failed) begin
      $display("PASSED");
    end
  end
endmodule
