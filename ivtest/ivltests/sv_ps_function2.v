// Check that it is possible to call a package scoped function with no
// arguments.

package P;
  function integer T();
    return 1;
  endfunction
endpackage

module test;
  initial begin
    integer x;
    x = P::T();
    if (x === 1) begin
      $display("PASSED");
    end else begin
      $display("FAILED: x = %d, expected 1", x);
    end
  end

endmodule
