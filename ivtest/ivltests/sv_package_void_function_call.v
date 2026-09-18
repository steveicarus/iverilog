// Check that package-scoped void functions can be called as statements.

package p;
  integer value = 0;

  function void add(input integer increment);
    value += increment;
  endfunction
endpackage

module test;
  initial begin
    p::add(17);
    p::add(25);

    if (p::value === 42) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end
endmodule
