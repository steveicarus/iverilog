// Check that the parameter to a method on a package scoped identifier is
// evaluated in the scope where the method is called, not where the identifier
// is declared.

package P;
  localparam N = 1;
  enum {
    A, B, C
  } e = A;
endpackage

module test;
  localparam N = 2;

  initial begin
    if (P::e.next(N) === P::C) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end
endmodule
