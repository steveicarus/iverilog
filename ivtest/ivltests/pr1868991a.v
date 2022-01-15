// pr1868991

module test();

   reg [31:0] mpr1[35-1:16];
   reg [29:0] dtlreq_addr;
   wire [7-1:0] select_mpr_inx = dtlreq_addr[7-1:0];
   wire [31:0]	select_dcs_mpr = mpr1[select_mpr_inx];

   integer	idx;
   initial begin
      for (idx = 16 ; idx < 35 ; idx = idx+1)
	mpr1[idx] = idx;

      for (idx = 16 ; idx < 35 ; idx = idx+1) begin
	 dtlreq_addr = idx;
	 #1 if (select_dcs_mpr !== idx) begin
	    $display("FAILED - select_dcs_mpr=%d, idx=%d", select_dcs_mpr, idx);
	    $finish;
	 end
      end

      $display("PASSED");
   end
endmodule
