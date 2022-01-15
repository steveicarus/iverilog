module main;

   reg [4:0] val;

   ornor4 dut (.O_OR(o_or), .O_NOR(o_nor),
	       .I0(val[0]), .I1(val[1]), .I2(val[2]), .I3(val[3]));

   initial begin
      for (val = 0 ;  val[4] == 0 ;  val = val+1) begin
	 #1 if (o_or !== |val[3:0]) begin
	    $display("FAILED -- |%b --> %b", val[3:0], o_or);
	    $finish;
	 end

	 if (o_nor !== ~|val[3:0]) begin
	    $display("FAILED -- ~|%b --> %b", val[3:0], o_nor);
	    $finish;
	 end
      end // for (val = 0 ;  val[4] == 0 ;  val = val+1)

      $display("PASSED");
   end // initial begin

endmodule // main
