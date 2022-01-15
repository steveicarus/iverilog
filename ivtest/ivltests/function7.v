module test;
   parameter BYTES = 2;
   localparam mxbit = BYTES*8-1;

   function integer clog2;
      input value;
      integer value;
      for (clog2=0; value>0; clog2=clog2+1) value = value >> 1;
   endfunction

   // This is not recognized as a constant function call!
   localparam cntrw = clog2(mxbit);

   integer    tmp;
   initial begin
      tmp = mxbit;
      $display("The maximum bit is %0d and uses a %0d bit counter", mxbit, cntrw);
      $display("clog2 does works here! Got %0d, should be %0d.", clog2(mxbit), clog2(tmp));

      if (cntrw !== clog2(tmp)) begin
	 $display("FAILED -- cntrw=%0d", cntrw);
	 $finish;
      end

      $display("PASSED");
      $finish;
   end
endmodule
