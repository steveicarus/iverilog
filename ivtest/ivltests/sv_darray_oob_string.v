// Check that out-of-bounds access on a string typed dynamic array works and
// returns the right value.

module test;

  string d[];
  string x;

  initial begin
    x = d[1];
    if (x == "") begin
      $display("PASSED");
    end else begin
      $display("FAILED. Expected '', got '%s'", x);
    end
  end

endmodule
