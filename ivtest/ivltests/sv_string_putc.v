// Check the string putc() task.

module test;

  string s;
  reg failed;

  initial begin
    failed = 1'b0;
    s = "abc";

    s.putc(1, 88);
    if (s != "aXc") begin
      $display("FAILED: expected aXc, got %s", s);
      failed = 1'b1;
    end

    s.putc(0, 89);
    if (s != "YXc") begin
      $display("FAILED: expected YXc, got %s", s);
      failed = 1'b1;
    end

    s.putc(3, 90);
    if (s != "YXc") begin
      $display("FAILED: expected YXc after out-of-range putc, got %s", s);
      failed = 1'b1;
    end

    s.putc(-1, 87);
    if (s != "YXc") begin
      $display("FAILED: expected YXc after negative putc, got %s", s);
      failed = 1'b1;
    end

    if (!failed) $display("PASSED");
  end

endmodule
