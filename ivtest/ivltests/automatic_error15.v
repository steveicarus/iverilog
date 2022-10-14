// Check that it is not possible to perform non-blocking assignments to a class
// object variable with automatic lifetime.

module test;

  class C;
  endclass

  task automatic auto_task;
    C c1, c2;
    c1 <= c2;
    $display("FAILED");
  endtask

  initial begin
    auto_task;
  end

endmodule
