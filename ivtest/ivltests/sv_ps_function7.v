// Check that the width of a package scoped function is reported correctly.

package P;
  function bit [22:0] s();
    return 0;
  endfunction
endpackage

module test;

  initial begin
    if ($bits(P::s()) == 23) begin
      $display("PASSED");
    end else begin
      $display("FAILED $bits(P::s()) = %0d", $bits(P::s()));
    end
  end

endmodule
