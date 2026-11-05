// Check that a ?: with $sformatf() on one side and a string literal on the
// other is a string expression.
module main;
  int n = 3;
  bit ok = 1;
  string s;

  initial begin
    s = ok ? $sformatf("PASS, n %0d", n) : "FAIL";
    if (s != "PASS, n 3") begin
      $display("FAILED: got '%s'", s);
      $finish;
    end

    s = !ok ? $sformatf("PASS, n %0d", n) : "FAIL";
    if (s != "FAIL") begin
      $display("FAILED: got '%s'", s);
      $finish;
    end

    s = ok ? "FAIL" : $sformatf("PASS, n %0d", n);
    if (s != "FAIL") begin
      $display("FAILED: got '%s'", s);
      $finish;
    end

    s = !ok ? "FAIL" : $sformatf("PASS, n %0d", n);
    if (s != "PASS, n 3") begin
      $display("FAILED: got '%s'", s);
      $finish;
    end

    $display("%s", ok ? $sformatf("PASS, n %0d", n) : "FAIL");
    $display("PASSED");
  end
endmodule
