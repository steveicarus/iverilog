// Check that variable initialization as part of the declaration works as
// expected in class methods.

module test;
  class C;
    task t(bit check);
      int x = 10; // The initialization should happen on each invocation
      if (check) begin
        if (x === 10) begin
          $display("PASSED");
        end else begin
          $display("FAILED");
        end
      end
      x = 20;
    endtask
  endclass

  initial begin
    C c;
    c = new;
    c.t(1'b0);
    c.t(1'b1);
  end
endmodule
