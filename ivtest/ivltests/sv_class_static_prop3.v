// Check that static class properties can be accessed for read and write on a
// class object. Check the property is shared across all instances and has the
// same value for all instances.

class C;
  static int i;
endclass

module test;

  C c1 = new;
  C c2 = new;
  C c3 = new;

  task t(C c);
    int x;
    x = c.i;
    c.i = x + 1;
  endtask

  initial begin
    t(c1);
    t(c2);
    if (c1.i == 2 && c2.i == 2 && c3.i == 2) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
