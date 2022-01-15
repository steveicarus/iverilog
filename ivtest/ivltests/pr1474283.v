module test;

   reg [2:0] tmp1;
   integer   tmp2;
   real      tmp3;

   initial begin
      t1(tmp1, 1);
      if (tmp1 !== 2) begin
	 $display("FAILED -- tmp1=%b", tmp1);
	 $finish;
      end

      t2(tmp2, 4);
      if (tmp2 !== 6) begin
	 $display("FAILED == tmp2=%d", tmp2);
	 $finish;
      end

      t3(tmp3, 0.5);
      if (tmp3 != 2.0) begin
	 $display("FAILED -- tmp3=%f", tmp3);
	 $finish;
      end

      $display("PASSED");
   end

   task t1(output [2:0] o, input [2:0] i);
      begin
	 o = i + 1;
      end
   endtask // tt

   task t2(output integer o, input integer i);
      o = i + 2;
   endtask // t2

   task t3(output real o, input real i);
      o = i + 1.5;
   endtask // t3

endmodule
