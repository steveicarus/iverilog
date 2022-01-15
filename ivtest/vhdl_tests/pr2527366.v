module test(input b);
  a ua(.BISTEA(b), .BISTEB(b));
endmodule // test

module a (input BISTEA, input BISTEB);
endmodule // a
