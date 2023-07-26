// Check that const variables in block scope are supported.

module test;

  initial begin
    const static integer x = 10;
    // The initializer expression is allowed to reference other const variables.
    const static integer y = 20 + x;

    if (x === 10 && y === 30) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
