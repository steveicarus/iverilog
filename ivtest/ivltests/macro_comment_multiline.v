// Check that multi-line comments in macros are supported. Including some corner
// cases like another comment start in the comment.

`define test(a, b, c) \
  (a + /* these is some \
   multi line */ b /* comment \
   that goes on \
   and on */ ) /* and some more \
\
   // and even has a comments \
   /* in the comment */ * c

module test;

  initial begin
    if (`test(1, 2, 3) === 9) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
