// Check that an error is reported when trying to call a package scoped task as
// a function.

package P;
  task t(int x);
  endtask
endpackage

module test;

  initial begin
    int y;
    y = P::t(10); // This should fail, t is a task
    $display("FAILED");
  end

endmodule
