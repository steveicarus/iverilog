// Copyright (c) 2026 Srinivasan Venkataramanan (AsFigo, https://www.asfigo.com)
// Check that a case statement with a string selector compiles and runs,
// and that the selector expression is evaluated exactly once per case
// statement (not once per guard).  GitHub issue #1751.
module test;

  function automatic int classify(string s);
    case (s)
      "NONE"   : return 1;
      "LOW"    : return 2;
      "MEDIUM" : return 3;
      default  : return 0;
    endcase
  endfunction

  // Side-effect counter: verify selector function is called only once.
  int call_count;

  function automatic string get_level();
    call_count += 1;
    return "MEDIUM";
  endfunction

  int errors = 0;

  initial begin
    // Basic functional checks.
    if (classify("NONE")   != 1) errors += 1;
    if (classify("LOW")    != 2) errors += 1;
    if (classify("MEDIUM") != 3) errors += 1;
    if (classify("HIGH")   != 0) errors += 1;

    // Selector must be evaluated exactly once regardless of guard count.
    call_count = 0;
    case (get_level())
      "NONE"   : errors += 1;
      "LOW"    : errors += 1;
      "MEDIUM" : ;
      default  : errors += 1;
    endcase
    if (call_count != 1) errors += 1;

    if (errors == 0)
      $display("PASSED");
    else
      $display("FAILED: %0d errors", errors);
    $finish(0);
  end
endmodule
