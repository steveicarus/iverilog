// pr1868991b

module test();

   reg [31:0] mpr1[35-1:16];
   reg [29:0] dtlreq_addr;
   wire [7-1:0] select_mpr_inx = dtlreq_addr[7-1:0];
   wire [31:0]	select_dcs_mpr = mpr1[select_mpr_inx];

endmodule
