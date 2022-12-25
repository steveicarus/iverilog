// Check that typed constructor calls are supported when assigning to an
// variable of a base class type.

module test;

  class B;
    int x = 0;

    task check;
      if (x === 10) begin
        $display("PASSED");
      end else begin
        $display("FAILED");
      end
    endtask
  endclass

  class C extends B;
    function new;
      x = 10;
    endfunction
  endclass

  initial begin
    B b;
    b = C::new;
    b.check;
  end

endmodule
