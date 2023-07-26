// Check that const variables are supported in the unit scope.

const integer x = 10;

// The initializer expression is allowed to reference other const variables.
const integer y = 20 + x;

module test;

  initial begin
    if (x === 10 && y === 30) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
