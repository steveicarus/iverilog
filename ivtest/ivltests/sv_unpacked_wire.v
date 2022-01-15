
module main;

   localparam width = 8;

   reg clk;
   reg [1:0] addr;
   logic [width-1:0] data [0:3];

   reg [width-1:0]  Q;

   // Does SystemVerilog support continuous assignment
   // of unpacked arrays? I think it does, but the LRM
   // is really not clear on this.
   wire [width-1:0]  data_x[0:3];
   assign data_x = data;
   always @(posedge clk)
     Q <= data_x[addr];

   reg [2:0]	   idx;
   initial begin
      clk = 0;
      data[0] = 0;
      data[1] = 1;
      data[2] = 2;
      data[3] = 3;
      addr = 0;

      for (idx = 0 ; idx < 4 ; idx += 1) begin
	 clk = 0;
	 #1 addr = idx[1:0];
	 #1 clk = 1;
	 #1 if (Q !== data[addr]) begin
	    $display("FAILED -- data[%0d]==%h, Q==%h", addr, data[addr], Q);
	    $finish;
	 end
      end

      $display("PASSED");
   end // initial begin

endmodule // main
