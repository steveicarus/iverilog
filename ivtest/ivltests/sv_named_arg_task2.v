// Check that binding task arguments by name is supported and that a mix of
// positional and named arguments is supported.

module test;

  task t(integer a, integer b, integer c);
    if (a == 1 && b == 2 && c == 3) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  endtask

  initial begin
    t(1, .c(3), .b(2));
  end

endmodule
