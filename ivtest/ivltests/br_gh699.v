
// This should generate an error reporting the undef_func, but not segfault.
module test1;
   function [31:0] func1 (input [31:0] x);
      return undef_func(x);
   endfunction : func1

   localparam X = func1(1);
endmodule : test1
