// Check that forwarding base class constructor arguments through the `extends`
// works when the derived class has an explicit constructor.

module test;

  class C;
    function new(int a);
      if (a == 10) begin
        $display("PASSED");
      end else begin
        $display("FAILED");
      end
    endfunction
  endclass

  // D has an explicit constructor
  class D extends C(10);
    int x;
    function new;
      x = 10;
    endfunction
  endclass

  D d = new;

endmodule
