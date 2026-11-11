// Check that assigning packed vectors cast to strings works with
// intra-assignment delays and event controls.

module test;

  bit failed = 1'b0;
  string value;
  reg [15:0] source = 16'h4142;
  event update;

  `define check(val) \
    if (val != "AB") begin \
      $display("FAILED(%0d). '%s' expected AB, got '%s'", `__LINE__, \
               `"val`", val); \
      failed = 1'b1; \
    end

  initial begin
    value = #1 string'(source);
    `check(value);

    value = "";
    fork
      value = @(update) string'(source);
      #1 -> update;
    join
    `check(value);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
