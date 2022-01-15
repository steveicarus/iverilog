
module TEST
  #(parameter ME = 0)
   (input OE,
    output wire [3:0] Q
    /* */);

   assign Q = OE? ME : 4'd0;

endmodule // TEST

module main;

   logic OE;
   logic [3:0] Q [0:3];

   genvar gidx;

   for (gidx = 0 ; gidx < 4 ; gidx = gidx+1) begin : DRV
      TEST #(.ME(gidx)) dut (.OE(OE), .Q(Q[gidx]));
   end

   int idx;
   initial begin
      OE = 1;
      #1 ;
      for (idx = 0 ; idx < 4 ; idx = idx+1) begin
	 if (Q[idx] !== idx[3:0]) begin
	    $display("FAILED -- Q[%0d] === %b", idx, Q[idx]);
	    $finish;
	 end
      end
      $display("PASSED");
   end

endmodule // main
