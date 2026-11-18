// Check that blocking assignments to string variables work with
// intra-assignment delays and event controls.

module test;

  bit failed = 1'b0;
  string value;
  event update;

  `define check(val) \
    if (val != "AB") begin \
      $display("FAILED(%0d). '%s' expected AB, got '%s'", `__LINE__, \
               `"val`", val); \
      failed = 1'b1; \
    end

  initial begin
    value = #1 "AB";
    `check(value);

    value = "";
    fork
      value = @(update) "AB";
      #1 -> update;
    join
    `check(value);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
