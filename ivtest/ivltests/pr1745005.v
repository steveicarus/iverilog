`begin_keywords "1364-2005"
// pr1745005
//
module main;

   reg [31:0] ref;
   reg [3:0]  addr;

   wire [3:0] out_net1 = ref[{addr,2'b00} +: 4];
   wire [3:0] out_net2 = ref[{addr,2'b11} -: 4];
   reg [3:0]  out_reg;

   initial begin
      ref = 32'h76543210;
      for (addr = 0 ;  addr < 8 ;  addr = addr+1) begin
	 #1 ;
	 out_reg = ref[{addr,2'b00} +: 4];
	 if (out_reg !== addr) begin
	    $display("FAILED -- addr=%d, out_reg=%b", addr, out_reg);
	    $finish;
	 end

	 if (out_net1 !== addr) begin
	    $display("FAILED -- addr=%d, out_net1=%b", addr, out_net1);
	    $finish;
	 end

	 if (out_net2 !== addr) begin
	    $display("FAILED -- addr=%d, out_net2=%b", addr, out_net2);
	    $finish;
	 end
      end
      $display("PASSED");
   end

endmodule
`end_keywords
