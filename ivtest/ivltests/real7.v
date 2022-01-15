// Verify that icarus can handle real compare versus int constant
module test ();

real myv;
parameter myp = 1.0;

initial
  begin
   myv = 1.0;
   if(myv <= 1)
     $display("PASSED");
   else
     $display("FAILED");

  end
endmodule
