// Check that const variables are supported in function and task scope.

module test;

  function automatic integer f(integer x);
    // Automatic const variables can have a non-const initializer epxression
    const integer y = 2 * x;
    return y;
  endfunction

  task automatic t(input integer x, output integer y);
    // Automatic const variables can have a non-const initializer epxression
    const integer z = 2 * x;
    y = z;
  endtask

  initial begin
    integer y;

    t(15, y);

    if (f(10) === 20 && y === 30) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
