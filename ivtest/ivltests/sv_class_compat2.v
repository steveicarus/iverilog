// Check that an object can be assigned to a variable of base class across more
// than one hierarchy level

module test;

  class B;
    int x;

    task t;
      $display("PASSED");
    endtask
  endclass

  class C extends B;
    int y;

    task t;
      $display("FAILED");
    endtask
  endclass

  class D extends C;
    int z;

    task t;
      $display("FAILED");
    endtask
  endclass

  B b;
  D d;

  initial begin
    d = new;
    b = d;

    b.t;
  end

endmodule
