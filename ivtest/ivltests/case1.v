module main;

   reg [3:0] cond;
   reg [2:0] t;

   always @*
     case (cond&4'b1110)
       'h0: t = 7;
       'h2: t = 6;
       'h4: t = 5;
       'h6: t = 4;
       'h8: t = 3;
       'ha: t = 2;
       'hc: t = 1;
       'he: t = 0;
     endcase

   integer   i;
   initial begin

      for (i = 0 ;  i < 8 ;  i = i + 1) begin
	 cond = i << 1;
	 #1 if (t !== (7 - i)) begin
	    $display("FAILED -- i=%d, cond=%b, t=%b", i, cond, t);
	    $finish;
	 end
      end

      $display("PASSED");
   end

endmodule // main
