// Check that forwarding base class constructor arguments through the `extends`
// works when the derived class has no constructor.

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

  // D has no constructor
  class D extends C(10);
  endclass

  D d = new;

endmodule
