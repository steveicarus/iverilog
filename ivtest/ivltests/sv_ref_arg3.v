// Two ref arguments of one call may name the same variable. A write
// through one is seen through the other.

module top;
  int a, b;

  task automatic update(ref int x, ref int y, input bit same);
    int old_y;
    old_y = y;
    x = 5;
    if (same && y !== 5) begin
      $display("FAILED: y=%0d after x=5", y);
      $finish;
    end
    if (!same && y !== old_y) begin
      $display("FAILED: y=%0d changed by x=5", y);
      $finish;
    end
    y = y + 1;
    if (same && x !== 6) begin
      $display("FAILED: x=%0d after y=y+1", x);
      $finish;
    end
    if (!same && x !== 5) begin
      $display("FAILED: x=%0d changed by y=y+1", x);
      $finish;
    end
  endtask

  initial begin
    a = 0;
    update(a, a, 1);
    if (a !== 6) begin
      $display("FAILED: a=%0d (expect 6)", a);
      $finish;
    end

    // Different variables do not affect each other.
    a = 0;
    b = 100;
    update(a, b, 0);
    if (a !== 5 || b !== 101) begin
      $display("FAILED: a=%0d b=%0d (expect 5, 101)", a, b);
      $finish;
    end
    $display("PASSED");
  end
endmodule
