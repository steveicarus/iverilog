// Check that a queue return type is supported for functions

module test;

  typedef int Q[$];

  // Since this is not an automatic function calling this repeatetly will
  // append to the same queue.
  function Q f1(int x);
    f1.push_back(1 + x);
    f1.push_back(2 + x);
  endfunction

  // Since this function is automatic a new queue will be created each time it
  // is called.
  function automatic Q f2(int x);
    f2.push_back(1 + x);
    f2.push_back(2 + x);
  endfunction

  initial begin
    Q a, b, c, d;

    a = f1(0);
    // `a` should be a copy and not affected by the second call
    b = f1(2);

    c = f2(0);
    d = f2(2);

    if (a.size() == 2 && a[0] == 1 && a[1] == 2 &&
        b.size() == 4 && b[0] == 1 && b[1] == 2 && b[2] == 3 && b[3] == 4 &&
        c.size() == 2 && c[0] == 1 && c[1] == 2 &&
        d.size() == 2 && d[0] == 3 && d[1] == 4) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
