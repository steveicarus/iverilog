// Check that a compilation-unit task takes precedence over a task in an
// enclosing instance.

integer result;

module child;

  initial begin
    result = 0;
    value();

    if (result == 1) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule

task value;
  result = 1;
endtask

module test;

  child i_child();

  task value;
    result = 2;
  endtask

endmodule
