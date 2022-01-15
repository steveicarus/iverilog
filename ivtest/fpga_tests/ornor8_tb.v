module main;

   reg [8:0] val;

   ornor8 dut (.O_OR(o_or), .O_NOR(o_nor),
	       .I0(val[0]), .I1(val[1]), .I2(val[2]), .I3(val[3]),
	       .I4(val[4]), .I5(val[5]), .I6(val[6]), .I7(val[7]));

   initial begin
      for (val = 0 ;  val[8] == 0 ;  val = val+1) begin
	 #1 if (o_or !== |val[7:0]) begin
	    $display("FAILED -- |%b --> %b", val[7:0], o_or);
	    $finish;
	 end

	 if (o_nor !== ~|val[7:0]) begin
	    $display("FAILED -- ~|%b --> %b", val[7:0], o_nor);
	    $finish;
	 end
      end

      $display("PASSED");
   end // initial begin

endmodule // main
