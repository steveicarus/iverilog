// Check that the parameter to a class method on a package scoped identifier is
// evaluated in the scope where the method is called, not where the identifier
// is declared.

package P;
  localparam N = 1;

  class C;
    function int f(int x);
      return x;
    endfunction
  endclass

  C c = new;
endpackage

module test;
  localparam N = 2;

  initial begin
    if (P::c.f(N) === 2) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end
endmodule
