// Check that it is not possible to perform non-blocking assignments to fields
// of structs with automatic lifetime.

module test;

  task automatic auto_task;
    struct packed {
      logic x;
    } s;
    s.x <= 10;
    $display("FAILED");
  endtask

  initial begin
    auto_task;
  end

endmodule
