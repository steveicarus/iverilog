// Check that const variables are supported in a package scope.

package P;
  const integer x = 10;

  // The initializer expression is allowed to reference other const variables.
  const integer y = 20 + x;
endpackage

module test;

  initial begin
    if (P::x === 10 && P::y === 30) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
