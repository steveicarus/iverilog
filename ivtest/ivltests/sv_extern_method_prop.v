// Check that an extern class method can use a property that has an
// initializer.

module test;
  class A;
    int x = 3;
    extern function int twice();
  endclass

  function int A::twice();
    return 2 * x;
  endfunction

  A a = new;

  initial begin
    if (a.twice() === 6)
      $display("PASSED");
    else
      $display("FAILED: twice=%0d", a.twice());
  end
endmodule
