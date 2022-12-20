// This is just a syntax test.

/* comment */ `default_nettype /* comment */ // comment
/* comment */ wire /* comment */ // comment
module m1;
endmodule

/* comment */`default_nettype/*
   comment */
/* comment */tri0/*
   comment */
module m2;
endmodule

module test;

initial $display("PASSED");

endmodule
