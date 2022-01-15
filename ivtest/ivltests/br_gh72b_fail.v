module modname;
`define macro1(arg1) $display(`"arg1`");
`define macro2(arg1=d1, arg2) $display(`"arg1 arg2`");
   initial begin
      `macro1(1,2)      // should fail
      `macro1(,2)       // should fail

      `macro2()         // should fail
      `macro2(1)        // should fail
      `macro2(1,2,3)    // should fail
   end
endmodule
