// Check that it is possible to call a class task without parenthesis after the
// task name when using the implicit `this` class handle.

module test;

  class C;
    task a;
      $display("PASSED");
    endtask

    task b;
      this.a;
    endtask
  endclass

  C c = new;

  initial begin
    c.b;
  end

endmodule
