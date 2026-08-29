module test;
  string a = "A";
  string b = "B";

  task automatic check(input int verbosity, input string expected);
    string color;
    color = (verbosity >= 3) ? a : b;
    if (color != expected) begin
      $display("FAILED: got '%s', expected '%s'", color, expected);
      $finish;
    end
  endtask

  initial begin
    check(3, "A");
    check(2, "B");
    $display("PASSED");
  end
endmodule
