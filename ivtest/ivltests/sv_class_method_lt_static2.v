// Check that specifing static lifetime for a class method function results in
// an error.

module test;

  class C;
    // This should fail, all class methods have automatic lifetime
    function static int t(int x);
      int y;
      y = 2 * x;
      return y;
    endfunction
  endclass

  initial begin
    $display("FAILED");
  end

endmodule
