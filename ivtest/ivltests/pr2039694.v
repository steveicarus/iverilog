module test;
   parameter foo = 5;
   initial $display("%m foo = %d", foo);
endmodule

module test_defparam;
   test U_test();
   defparam U_test.foo = 2;
endmodule

module test_inline;
   test #(.foo(2)) U_test();
endmodule

module testcase_defparam;
   test_defparam test_defparam_a();
   test_defparam test_defparam_b();
   test_defparam test_defparam_c();
endmodule

module testcase_inline;
   test_inline test_inline_a();
   test_inline test_inline_b();
   test_inline test_inline_c();
endmodule
