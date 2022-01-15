module modname;
`define macro1(arg1=d1) $display(`"arg1`");
`define macro2(arg1=d1, arg2=d2) $display(`"arg1 arg2`");
   initial begin
      `macro1() // Works
      `macro1(1) // Works

      `macro2() // Cause wrong number of arguments error
      `macro2(1) // Cause wrong number of arguments error
      `macro2(1,2) // Works

      `macro2(,) // Works
      `macro2(1,) // Works
      `macro2(1,2) // Works

      `macro2(,2) // Works
   end
endmodule
