// Check that nonempty queues print with an assignment-pattern prefix.

module test;
  int values[$];
  int value;
  string text;
  bit failed = 1'b0;

  `define check(fmt, val, exp) do begin \
    text = $sformatf(fmt, val); \
    if (text != exp) begin \
      $display("FAILED(%0d). '%s' expected %s, got %s", `__LINE__, \
               `"val`", exp, text); \
      failed = 1'b1; \
    end \
  end while (0)

  initial begin
    values.push_back(1);
    `check("%p", values, "'{1}");
    `check("%P", values, "'{1}");
    `check("%0p", values, "'{1}");
    `check("%0P", values, "'{1}");

    values.push_back(2);
    `check("%p", values, "'{1, 2}");
    value = values.pop_back();
    value = values.pop_back();
    `check("%p", values, "{}");
    `check("%0p", values, "{}");

    if (!failed) begin
      $display("PASSED");
    end
  end
endmodule
