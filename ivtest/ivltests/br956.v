package test_pkg;

// Need to add enumerations to packages.
   typedef enum logic[4:0] {

      EXC_A                = 0,
      EXC_B                = 1,
      EXC_C                = 2
   } exc_code_t;

// Need to search up the parent scope searching for the enum definition.
   function logic func1(exc_code_t c);
      logic rVal;
      case(c)
         EXC_C  : rVal = 1;
         default: rVal = 0;
      endcase
      return(rVal);
   endfunction

endpackage

module a();

   import test_pkg::func1;
   import test_pkg::exc_code_t;

   exc_code_t exc_code;
   logic result;

   initial begin

// Need to compare the base enumeration definition to check compatibility.
      exc_code = test_pkg::EXC_C;

      result = func1(exc_code);

      if(result==1'b1) begin
         $display("PASSED");
         $finish;
      end

      $display("FAILED");
      $finish;

   end

endmodule
