module main;

   reg [3:0] cond;
   reg [2:0] t, q;

   always @* begin
     case (cond&4'b1110)
       'h0: t = 7;
       'h2: t = 6;
       'h4: t = 5;
       'h6: t = 4;
       'h8: t = 3;
       'ha: t = 2;
       'hc: t = 1;
       'he: t = 0;
     endcase // case(cond&4'b1110)

      q = ~t;

   end // always @ *

   integer   i;
   initial begin

      for (i = 0 ;  i < 8 ;  i = i + 1) begin
	 cond = i << 1;
	 #1 if (q !== ( 3'b111 & ~(7 - i))) begin
	    $display("FAILED -- i=%d, cond=%b, q=%b", i, cond, q);
	    $finish;
	 end
      end

      $display("PASSED");
   end

endmodule // main
