module main;

   reg [3:0] count;
   reg	     CLOCK;
   reg	     RSTn, SETn;

   (* ivl_synthesis_off *)
   initial begin
      CLOCK = 0;
      RSTn = 0;
      SETn = 1;

      #1 CLOCK = 1;
      #1 CLOCK = 0;

      if (count !== 4'b0000) begin
	 $display("FAILED -- initial reset doesn't");
	 $finish;
      end

      RSTn = 1;
      #1 CLOCK = 1;
      #1 CLOCK = 0;
      #1 CLOCK = 1;
      #1 CLOCK = 0;

      if (count !== 4'b0010) begin
	 $display("FAILED -- count up is %b", count);
	 $finish;
      end

      SETn = 0;
      #1 ;

      if (count !== 4'b1101) begin
	 $display("FAILED -- Aset failed: count=%b", count);
	 $finish;
      end

      SETn = 1;
      #1 CLOCK = 1;
      #1 CLOCK = 0;

      if (count !== 4'b1110) begin
	 $display("FAILED -- Aset didn't release: count=%b", count);
	 $finish;
      end

      RSTn = 0;
      #1 ;
      if (count !== 4'b0000) begin
	 $display("FAILED -- Aclr failed: count=%b", count);
	 $finish;
      end

      $display("PASSED");
      $finish;
   end

   (* ivl_synthesis_on *)
   always @(posedge CLOCK or negedge RSTn or negedge SETn)
     begin
	if (!RSTn)
	  count =0; //async clear
	else
	  if (!SETn)
	    count = 4'b1101;  //async set
	  else
	    count = count + 1;
     end

endmodule
