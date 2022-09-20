// Check that it is possible to call a package scoped function with empty
// positional arguments.

package P;
  function integer T(integer x = 1, integer y = 2);
    return x + y;
  endfunction
endpackage

module test;
  initial begin
    integer x;
    x = P::T(, 4);
    if (x === 5) begin
      $display("PASSED");
    end else begin
      $display("FAILED. x = %d, expected 5", x);
    end
  end

endmodule
