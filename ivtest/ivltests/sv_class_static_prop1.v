// Check that static class properties can be accessed for read and write in
// class tasks. Check the property is shared across all instances and has the
// same value for all instances.

class C;
  static int i;
  task t;
    int x;
    x = i;
    i = x + 1;
  endtask
endclass

module test;

  C c1 = new;
  C c2 = new;
  C c3 = new;

  initial begin
    c1.t();
    c2.t();
    if (c1.i == 2 && c2.i == 2 && c3.i == 2) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
