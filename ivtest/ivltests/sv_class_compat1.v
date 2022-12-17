// Check that an object can be assigned to a variable of its base class type

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

  B b;
  C c;

  initial begin
    c = new;
    b = c;

    b.t;
  end

endmodule
