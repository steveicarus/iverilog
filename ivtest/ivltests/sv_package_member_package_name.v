// Check that a package member can have the same name as another package.

package p;
  typedef logic [3:0] q;
endpackage

package q;
endpackage

module test;
  p::q value;

  initial begin
    value = 4'ha;
    if ($bits(value) == 4 && value === 4'ha)
      $display("PASSED");
    else
      $display("FAILED");
  end
endmodule
