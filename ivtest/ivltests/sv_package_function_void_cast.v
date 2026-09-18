// Check void casts of package-scoped non-void function calls.

package p;
  integer value = 0;

  function integer set(input integer new_value);
    value = new_value;
    set = value;
  endfunction
endpackage

module test;
  initial begin
    void'(p::set(42));

    if (p::value === 42) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end
endmodule
