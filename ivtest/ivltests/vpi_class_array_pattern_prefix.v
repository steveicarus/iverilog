// Check that array and queue properties print with an assignment-pattern prefix.

class C;
  int values[];
  int queue[$];
endclass

module test;
  C object;
  int values[];
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
    object = new;
    values = new[2];
    values[0] = 1;
    values[1] = 2;
    object.values = values;
    object.queue.push_back(1);
    object.queue.push_back(2);

    `check("%p", object.values, "'{1, 2}");
    `check("%0p", object.values, "'{1, 2}");
    `check("%p", object.queue, "'{1, 2}");
    `check("%0p", object.queue, "'{1, 2}");

    if (!failed) begin
      $display("PASSED");
    end
  end
endmodule
