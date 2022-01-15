module top;
   real rval1=1.0, rval2=2.0;
   realtime rtval=1.0;

   initial begin
      $display("rval1=", rval1,,"rval2=", rval2,,"rtval=",rtval);
   end
endmodule
