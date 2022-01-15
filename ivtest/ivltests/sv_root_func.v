function int a_func (input int id);
   a_func = id;
endfunction

// This should print the following:
//    This is func 2.
//    This is func 1.
module top;
   initial begin
      $display("this is func %0d.", a_func(2));
      $display("this is func %0d.", a_func(1));
   end
endmodule
