// Check that an expression is correctly detected to contain an automatic
// variable if the variable is in a SystemVerilog size cast expression.

module automatic_error;

  reg g;

  task automatic auto_task;
    reg l;

    begin: block
      assign g = 1'(l);
    end
  endtask

  initial begin
    auto_task;
    $display("FAILED");
  end

endmodule
