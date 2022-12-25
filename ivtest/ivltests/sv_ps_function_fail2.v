// Check that trying to call a package scoped variable as a function results in
// an error.

package P;
  int x;
endpackage

module test;

  initial begin
    int y;
    y = P::x(10); // This should fail, x is not a function
    $display("FAILED");
  end

endmodule
