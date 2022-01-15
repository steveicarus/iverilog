module main;

   localparam BYTESIZE = 8;
   localparam STRLEN = 4;
   localparam [15:0] CHAR = "10";

   function [STRLEN*BYTESIZE - 1 : 0] bits2text;
      input [STRLEN-1:0] use_map;

      integer		 idx;
      begin
	 bits2text = 0;
	 for (idx = 0 ; idx < STRLEN ; idx = idx+1) begin
	    bits2text[(idx*BYTESIZE) +: BYTESIZE] = CHAR[BYTESIZE*use_map[idx] +: BYTESIZE];
	 end
      end
   endfunction


   localparam [STRLEN*BYTESIZE - 1 : 0] str0010 = bits2text(4'b0010);
   localparam [STRLEN*BYTESIZE - 1 : 0] str0100 = bits2text(4'b0100);
   localparam [STRLEN*BYTESIZE - 1 : 0] str0011 = bits2text(4'b0011);
   localparam [STRLEN*BYTESIZE - 1 : 0] str1100 = bits2text(4'b1100);

   reg [STRLEN*BYTESIZE - 1 : 0] tmp;
   initial begin
      tmp = bits2text(4'b0010);
      if (tmp !== str0010) begin
	 $display("FAILED -- str0010=%h, expect %h", str0010, tmp);
	 $finish;
      end

      tmp = bits2text(4'b0100);
      if (tmp !== str0100) begin
	 $display("FAILED -- str0100=%h, expect %h", str0100, tmp);
	 $finish;
      end

      tmp = bits2text(4'b0011);
      if (tmp !== str0011) begin
	 $display("FAILED -- str0011=%h, expect %h", str0011, tmp);
	 $finish;
      end

      tmp = bits2text(4'b1100);
      if (tmp !== str1100) begin
	 $display("FAILED -- str1100=%h, expect %h", str1100, tmp);
	 $finish;
      end

      $display("PASSED");
      $finish;
   end

endmodule // main
