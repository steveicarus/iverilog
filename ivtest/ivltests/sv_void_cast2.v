// Check that void casts on class methods are supported

module test;

  int a;
  real b;
  string c;

  class C;
    function int f1(int x);
      a = x;
      return x;
    endfunction

    function real f2(real x);
      b = x;
      return x;
    endfunction

    function string f3(string x);
      c = x;
      return x;
    endfunction
  endclass

  C d;

  initial begin
    d = new;
    void'(d.f1(10));
    void'(d.f2(1.0));
    void'(d.f3("10"));

    if (a === 10 && b == 1.0 && c == "10") begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
