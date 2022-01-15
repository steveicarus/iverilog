`begin_keywords "1364-2005"
module test;

   real tmp;

   function fun1;
      input [31:0] parm;
      real tmp;

      begin
	 tmp = 0.0;
	 fun1 = parm[0];
      end

   endfunction // fun1

   reg bit;
   reg [31:0] var;
   initial begin
      tmp = 1.0;
      var = 1;
      bit = fun1(var);

      if (tmp != 1.0) begin
	 $display("FAILED -- tmp=%f, fun1.tmp=%f", tmp, fun1.tmp);
	 $finish;
      end

      $display("PASSED");
   end

endmodule // test
`end_keywords
