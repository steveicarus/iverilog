// This is just a syntax test.

/* comment */ `resetall /* comment */ // comment
/* comment */ `celldefine /* comment */ // comment
module cell1;
endmodule
/* comment */ `endcelldefine /* comment */ // comment

/* comment */`resetall/*
   comment */
/* comment */`celldefine/*
   comment */
module cell2;
endmodule
/* comment */`endcelldefine/*
   comment */

module test;

initial $display("PASSED");

endmodule
