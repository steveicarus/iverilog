// Check that nonempty dynamic arrays print with an assignment-pattern prefix.

module test;
  int values[];
  real real_values[];
  string string_values[];
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
    values = new[0];
    `check("%p", values, "{}");
    `check("%0p", values, "{}");

    values = new[1];
    values[0] = 1;
    `check("%p", values, "'{1}");
    `check("%P", values, "'{1}");
    `check("%0p", values, "'{1}");
    `check("%0P", values, "'{1}");

    values = new[2];
    values[0] = 1;
    values[1] = 2;
    `check("%p", values, "'{1, 2}");

    real_values = new[2];
    real_values[0] = 1.5;
    real_values[1] = 2.5;
    `check("%p", real_values, "'{1.5, 2.5}");

    string_values = new[2];
    string_values[0] = "one";
    string_values[1] = "two";
    `check("%p", string_values, "'{\"one\", \"two\"}");

    if (!failed) begin
      $display("PASSED");
    end
  end
endmodule
