// Check break and continue from within case statements in repeat loops.

module test;

  integer selector;
  integer count;
  bit failed = 1'b0;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %0d, got %0d", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    selector = 0;
    count = 0;
    repeat (3) begin
      count = count + 1;
      case (selector)
        0: continue;
      endcase
    end
    `check(count, 3);

    count = 0;
    repeat (3) begin
      count = count + 1;
      case (selector)
        1: count = -100;
        default: continue;
      endcase
    end
    `check(count, 3);

    count = 0;
    repeat (3) begin
      count = count + 1;
      case (selector)
        0: break;
      endcase
    end
    `check(count, 1);

    count = 0;
    repeat (3) begin
      count = count + 1;
      case (selector)
        1: count = -100;
        default: break;
      endcase
    end
    `check(count, 1);

    count = 0;
    repeat (3) begin
      count = count + 1;
      casex (selector)
        0: break;
      endcase
    end
    `check(count, 1);

    count = 0;
    repeat (3) begin
      count = count + 1;
      casez (selector)
        0: case (selector)
          0: continue;
        endcase
      endcase
    end
    `check(count, 3);

    count = 0;
    repeat (3) begin
      case (selector)
        1: count = -100;
      endcase
      count = count + 1;
    end
    `check(count, 3);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
