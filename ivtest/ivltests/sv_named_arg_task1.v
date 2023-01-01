// Check that binding task arguments by name is supported.

module test;

  task t(integer a, integer b);
    if (a == 1 && b == 2) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  endtask

  initial begin
    t(.b(2), .a(1));
  end

endmodule
