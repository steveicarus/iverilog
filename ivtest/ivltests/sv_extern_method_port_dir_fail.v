// The definition must use the port directions of the prototype.

module test;
  class C;
    extern task t(input int a);
  endclass

  task C::t(output int a);
    a = 1;
  endtask
endmodule
