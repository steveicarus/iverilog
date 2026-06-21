// Check that using a class task as an expression reports an error.

module test;

  class C;
    task t;
    endtask
  endclass

  C c;

  initial begin
    int x;
    c = new;
    x = c.t();
    $display("FAILED");
  end

endmodule
