module a();
// Need to add enumerations to packages.
   typedef enum logic[4:0] {
      EXC_A                = 0,
      EXC_B                = 1,
      EXC_C                = 2
   } exc_code_t;
// Need to search up the parent scope searching for the enum definition.
   function exc_code_t func1(bit inx);
      exc_code_t rVal;
      case(inx)
         1 : rVal = EXC_C;
         0:  rVal = EXC_B;
         default: rVal = EXC_A;
      endcase
      return(rVal);
   endfunction
   exc_code_t exc_code;
   initial begin
// Need to compare the base enumeration definition to check compatibility.
      exc_code = func1(1'b1);
      if(exc_code== EXC_C) begin
         $display("PASSED");
         $finish;
      end
      $display("FAILED");
      $finish;
   end
endmodule
