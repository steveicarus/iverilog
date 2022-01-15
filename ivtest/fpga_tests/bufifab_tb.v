module main;

   reg [2:0] i;
   wire      out0, out1;
   wire      ref0, ref1;

   bufifab dut(.Out0(out0), .Out1(out1), .I(i[0]), .E(i[1]));

   bufif0 (ref0, i[0], i[1]);
   bufif1 (ref1, i[0], i[1]);
   initial begin
      i = 0;

      for (i = 0 ;  i[2] == 0 ;  i = i+1) begin
	 #1 $display("I=%b, E=%b, Out0=%b, Out1=%b", i[0], i[1], out0, out1);

	 if (out0 !== ref0) begin
	    $display("FAILED -- ref0=%b, out0=%b", ref0, out0);
	    $finish;
	 end

	 if (out1 !== ref1) begin
	    $display("FAILED -- ref1=%b, out1=%b", ref1, out1);
	    $finish;
	 end
      end // for (i = 0 ;  i[2] == 0 ;  i = i+1)

      $display("PASSED");
   end

endmodule // main
