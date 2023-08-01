// Check that forwarding base class constructor arguments through the `extends`
// works when the derived class has an implicit constructor.

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

  // D has an implicit constructor
  class D extends C(10);
    int y;
  endclass

  D d = new;

endmodule
