// Check that it is possible to exit from a task using the return statement with
// affecting other concurrently running instances of the same task.

module test;

  task automatic t(input integer a, output integer b);
    if (a == 0) begin
      b = 1;
      return;
    end
    #10
    b = 100;
  endtask

  integer b1;
  integer b2;

  initial begin
    fork
      t(0, b1);
      t(1, b2);
    join

    if (b1 == 1 && b2 == 100) begin
      $display("PASSED");
    end else begin
      $display("FAILED b1=%0d, b2=%0d", b1, b2);
    end
  end

endmodule
