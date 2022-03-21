`timescale 1ns / 1ps

module test();

localparam period = 20;

reg  clk;
reg  rst;

reg  [4:0]  waddr;
reg  [31:0] wdata;
reg  wvalid;
reg  wready;

always begin
  clk = 1'b0;
  #(period/2);
  clk = 1'b1;
  #(period/2);
end

always @(posedge clk) begin
  if (rst)
    wready <= 1'b0;
  else if (!wready && wvalid)
    wready <= 1'b1;
end

genvar b;

for (b = 0; b < 4; b = b + 1) begin : BYTE_BRAM_GEN
  wire [7:0] data_in;
  reg  [7:0] byte_ram [31:0];

  assign data_in = wdata[b*8 +: 8];

  always @(posedge clk) begin
    if (wvalid && wready)
      byte_ram[waddr] <= data_in;
  end
end

wire [7:0] my_byte = BYTE_BRAM_GEN[0].byte_ram[0];

task automatic wait_for_wready;

begin : waiting
  @(posedge wready);  // wait for rising edge
end

endtask

task init_memory;

begin
  @(posedge clk);

  waddr  <= 0;
  wdata  <= 1;
  wvalid <= 1'b1;

  wait_for_wready;

  @(posedge clk);
  @(posedge clk);

  $display("my_byte %h", my_byte);

  if (my_byte === 8'h01)
    $display("PASSED");
  else
    $display("FAILED");
end

endtask

initial begin
  wdata  = 32'd0;
  wvalid = 1'b0;

  rst = 1'b1;
  #period;
  rst = 1'b0;

  init_memory;

  $finish(0);
end

endmodule
