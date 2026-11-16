// Check string value parameters of a parameterized class: the default, a
// positional and a named override, equal strings sharing one specialization,
// and a static property kept per specialization.

class C #(parameter string S = "dflt", parameter int N = 1);
  static int cnt = 0;

  function string get();
    return S;
  endfunction
endclass

module test;

  C c0;
  C #("dflt") c1;
  C #("abc") c2;
  C #(.S("abc"), .N(1)) c3;
  C #("xyz", 2) c4;

  bit failed = 1'b0;

  task check(input string got, input string exp, input string msg);
    if (got != exp) begin
      $display("FAILED: %0s, got `%0s`, expected `%0s`", msg, got, exp);
      failed = 1'b1;
    end
  endtask

  initial begin
    c0 = new;
    c2 = new;
    c4 = new;

    // The same value gives the same class, so the handles are compatible.
    c1 = c0;
    c3 = c2;

    check(c1.get(), "dflt", "default value");
    check(c3.get(), "abc", "positional and named override");
    check(c4.get(), "xyz", "override with a second parameter");

    c2.cnt = 5;
    if (c3.cnt != 5 || c0.cnt != 0 || c4.cnt != 0) begin
      $display("FAILED: static property not kept per specialization");
      failed = 1'b1;
    end

    if (!failed)
      $display("PASSED");
  end

endmodule
