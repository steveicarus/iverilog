// Check that an error is reported when trying to call a package scoped function
// that does not exist.

package P;
endpackage

module test;

  initial begin
    int y;
    y = P::f(10); // This should fail, f does not exist
    $display("FAILED");
  end

endmodule
