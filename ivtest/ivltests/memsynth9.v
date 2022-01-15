module main;

   parameter CACHE_RAM = 128;
   parameter ADR_WIDTH = 7;

   reg [31:0] buff[0:CACHE_RAM], data_o, data_i;

   reg [ADR_WIDTH-1:0] addr;
   reg	      clk, rst, wr;

   (* ivl_synthesis_on *)
   always @(posedge clk)
     if (wr) buff[addr] <= data_i;

   (* ivl_synthesis_on *)
   always @(posedge clk or posedge rst)
     begin
	if (rst)
	  data_o <=  32'h0;
	else if (wr)
	  data_o <=  data_i;
	else
	  data_o <=  buff[addr];
     end

   (* ivl_synthesis_off *)
   initial begin
      clk = 0;
      rst = 0;
      wr = 1;
      for (addr = 0 ;  addr < 64 ;  addr = addr+1) begin
	 data_i <= addr;
	 #1 clk = 1;
	 #1 clk = 0;
	 if (data_o !== data_i) begin
	    $display("FAILED -- write addr=0x%h, data_o=%h", addr, data_o);
	    $finish;
	 end
      end

      wr = 0;
      data_i = 32'hx;
      for (addr = 0 ;  addr < 64 ;  addr = addr+1) begin
	 #1 clk = 1;
	 #1 clk = 0;
	 if (data_o !== addr) begin
	    $display("FAILED -- read addr=0x%h, data_o=%h", addr, data_o);
	    $finish;
	 end
      end

      $display("PASSED");
   end

endmodule // main
