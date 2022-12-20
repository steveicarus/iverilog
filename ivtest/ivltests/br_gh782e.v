// This is just a syntax test.

/* comment */ `begin_keywords /* comment */ // comment
/* comment */ "1364-2005" /* comment */ // comment
module m1;
endmodule
/* comment */ `end_keywords /* comment */ // comment

/* comment */`begin_keywords/*
   comment */
/* comment */"1364-2005"/*
   comment */
module m2;
endmodule
/* comment */`end_keywords/*
   comment */

module test;

initial $display("PASSED");

endmodule
