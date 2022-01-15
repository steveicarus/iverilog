module test();
   wire clk;
   reg  [8:0]	lowp2_tmp;
   reg  [8:0]	lowp2_out;
  always @(posedge clk) begin
     lowp2_out <= ( {lowp2_tmp[8], lowp2_tmp} ) >> 1;
  end
endmodule
