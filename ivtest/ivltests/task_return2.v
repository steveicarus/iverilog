// Check that it is possible to return from a named sub-block of a task using a
// `return` statement.

module test;

  task t(input integer a);
    begin : subblock
      if (a == 1) begin : condition
        return;
      end
    end
    $display("FAILED");
    $finish;
  endtask

  initial begin
    t(1);
    #10
    $display("PASSED");
  end

endmodule
