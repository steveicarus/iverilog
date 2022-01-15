module test (
input		clk_dma,
input		rst_dma_n,

input		wr_valid,
input		wr_trans,
input		wr_flush,

output		wr_ready);

wire   buf_wr_wstrb;

assign buf_wr_wstrb = wr_ready && wr_valid;
assign wr_ready = wr_flush ;

endmodule
