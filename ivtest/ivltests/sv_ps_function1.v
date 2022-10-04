// Check that it is possible to call a package scoped function.

package P;
  function integer T(integer x, integer y);
    return x + y;
  endfunction
endpackage

module test;
  initial begin
    integer x;
    x = P::T(1, 2);
    if (x === 3) begin
      $display("PASSED");
    end else begin
      $display("FAILED. x = %d, expected 3", x);
    end
  end

endmodule
