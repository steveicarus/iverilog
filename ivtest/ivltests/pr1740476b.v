module main;

   wire [3:0] src [15:0];
   wire [3:0] dst [15:0];

   genvar i;
   for (i = 0 ; i < 16; i = i+1) begin:bb
      buffer u (.out(dst[i]), .in(src[i]));
   end

   for (i = 0 ;  i < 16 ;  i = i+1) begin:drv
      assign src[i] = i;
   end

   integer   idx;
   initial begin
      #1 ;
      for (idx = 0 ;  idx < 16 ;  idx = idx+1) begin
	 if (dst[idx] !== idx) begin
	    $display("FAILED -- src[%0d]==%b, dst[%0d]==%b",
		     idx, src[idx], idx, dst[idx]);
	    $finish;
	 end
      end
      $display("PASSED");
   end

endmodule // main

module buffer (input wire [3:0] in, output wire [3:0] out);

   assign out = in;

endmodule // buffer
