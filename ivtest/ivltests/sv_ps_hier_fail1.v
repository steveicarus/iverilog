// Check that package scoped identifiers lookup does not cross the package
// boundary.

int x;

package P;
endpackage

module test;
  initial begin
    int y;
    y = P::x; // This should fail. x is visible from within the package,
              // but can't be accessed through a package scoped identifier.
    $display("FAILED");
  end
endmodule
