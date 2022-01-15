module main;

   reg [7:0] data_i;
   reg [2:0] addr;
   reg	     clk, rst, wr;

   reg [7:0] data_o, buff[0:7];

   (* ivl_synthesis_on *)
   always @(posedge clk or posedge rst)
     begin
	if (rst)
	  data_o <= 8'h0;
	else if (wr) begin
	   buff[addr] <= data_i;
	   data_o <= data_i;
	end else
	  data_o <= buff[addr];
     end

   (* ivl_synthesis_off *)
   initial begin
      clk = 0;
      rst = 1;
      wr = 1;
      addr = 0;
      data_i = 8'hff;

      #1 clk = 1;
      #1 clk = 0;
      if (data_o !== 8'h00) begin
	 $display("FAILED -- reset data_o=%b", data_o);
	 $finish;
      end

      rst = 0;
      wr = 1;

      for (addr = 0;  addr < 7; addr = addr+1) begin
	 data_i = addr;
	 #1 clk = 1;
	 #1 clk = 0;
	 if (data_o !== data_i) begin
	    $display("FAILED -- write data_i=%h, data_o=%h", data_i, data_o);
	    $finish;
	 end
      end

      wr = 0;
      data_i = 8'hff;

      for (addr = 0 ;  addr < 7;  addr = addr+1) begin
	 #1 clk = 1;
	 #1 clk = 0;
	 if (data_o !== {5'b00000, addr}) begin
	    $display("FAILED -- read addr=%h, data_o=%h", addr, data_o);
	    $finish;
	 end
      end

      $display("PASSED");
   end

endmodule // main
