// Check that an override of a virtual method must have the same
// argument directions.

module test;

  class Base;
    virtual task t(input int a, output int b);
      b = a;
    endtask
  endclass

  class Derived extends Base;
    virtual task t(input int a, input int b);
    endtask
  endclass

endmodule
