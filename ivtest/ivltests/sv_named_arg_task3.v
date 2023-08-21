// Check that binding task arguments by name is supported and that an empty
// value can be bound to the name, in which case the default argument value
// should be used.

module test;

  task t(integer a, integer b = 2);
    if (a == 1 && b == 2) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  endtask

  initial begin
    t(.a(1), .b());
  end

endmodule
