// Check that `super` keyword can be used to call tasks of the base class

class B;
  task t;
    $display("PASSED");
  endtask
endclass

class C extends B;
  task t;
    $display("FAILED");
  endtask

  task check;
    super.t;
  endtask
endclass

module test;
  C c;

  initial begin
    c = new;
    c.check;
  end
endmodule
