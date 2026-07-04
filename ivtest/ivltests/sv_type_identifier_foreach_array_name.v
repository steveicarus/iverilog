// Check that foreach array expressions can shadow visible type identifiers.

typedef int A;

module test;

  reg failed;

  initial begin
    failed = 1'b0;

    // The array declaration follows the foreach loop, so the lexer only sees
    // the visible typedef when parsing the array expression name.
    foreach (A[]) begin
      failed = 1'b1;
    end

    if (!failed) begin
      $display("PASSED");
    end
  end

  int unsigned A [3];

endmodule
