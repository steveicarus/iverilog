// Check that specifing static lifetime for a class method taks results in an
// error.

module test;

  class C;
    // This should fail, all class methods have automatic lifetime
    task static t(int x);
      int y;
      y = 2 * x;
    endtask
  endclass

  initial begin
    $display("FAILED");
  end

endmodule
