// Check that writes with invalid indices into class properties containing
// fixed unpacked arrays of class handles leave valid elements unchanged.

class item;
  int value;
endclass

class holder;
  item values[2];
endclass

module test;

  holder h;
  item first, second, selected;
  logic [64:0] index;
  bit failed = 1'b0;
  integer i;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %h, got %h", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    h = new;
    first = new;
    second = new;
    first.value = 11;
    second.value = 22;
    h.values[0] = first;
    h.values[1] = second;
    for (i = 0; i < 7; i = i + 1) begin
      case (i)
        0: index = 'x;
        1: index = 'z;
        2: index = 2;
        3: index = -1;
        4: index = 65'h100000000;
        5: index = 65'h10000000000000000;
        6: index = 65'bx1;
      endcase
      h.values[index] = null;
      selected = h.values[0];
      `check(selected != null, 1'b1);
      if (selected != null) begin
        `check(selected.value, 11);
      end
      selected = h.values[1];
      `check(selected != null, 1'b1);
      if (selected != null) begin
        `check(selected.value, 22);
      end
    end
    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
