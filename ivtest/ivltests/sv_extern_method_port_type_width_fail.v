// The definition must use the port types of the prototype.

module test;
  class C;
    extern task t(input logic [7:0] a);
  endclass

  task C::t(input logic [3:0] a);
  endtask
endmodule
