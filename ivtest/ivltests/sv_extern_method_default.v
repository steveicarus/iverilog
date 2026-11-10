// A default argument value given only by the prototype is used by calls.

module test;
  class C;
    extern function int d(int a = 3);
  endclass

  function int C::d(int a);
    return a;
  endfunction

  C c;
  initial begin
    c = new;
    if (c.d() === 3 && c.d(5) === 5)
      $display("PASSED");
    else
      $display("FAILED");
  end
endmodule
