// Check that `super` keyword can be used to call functions of the base class

class B;
  function int f;
    return 1;
  endfunction
endclass

class C extends B;
  function int f;
    return 2;
  endfunction

  task check;
    if (super.f() === 1) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  endtask
endclass

module test;
  C c;

  initial begin
    c = new;
    c.check;
  end
endmodule
