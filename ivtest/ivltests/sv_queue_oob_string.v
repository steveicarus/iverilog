// Check that out-of-bounds access on a string typed queue works and returns the
// right value.

module test;

  string q[$];
  string x;

  initial begin
    x = q[1];
    if (x == "") begin
      $display("PASSED");
    end else begin
      $display("FAILED. Expected '', got '%s'", x);
    end
  end

endmodule
