/* Extracted from PR#820. */

module main();
  wire clk;
  wire reset;
  reg [3:0] waddr, raddr;
  reg [7:0] wdata;
  wire [7:0] rdata;

  clk_reset_gen cg(clk, reset);

  ram_rw #(8,4) r(clk, waddr, wdata, 1'b1, raddr, rdata);

  initial begin
    waddr = 4'd0;
    raddr = 4'd14;
    wdata = 0;
    #3001;
    $finish(0);
  end

  always @(posedge clk) begin
    waddr <= #1 waddr + 1;
    raddr <= #1 raddr + 1;
    wdata <= #1 wdata + 3;
  end

  always @(posedge clk)
    $display($time,,"waddr wdata %d %d    raddr rdata %d %d",waddr,wdata,raddr,rdata);

endmodule

module ram_rw(clk,waddr,wdata,we,raddr,rdata);
  parameter WDATA = 8;
  parameter WADDR = 11;

  input clk;

  input [(WADDR-1):0] waddr;
  input [(WDATA-1):0] wdata;
  input we;

  input [(WADDR-1):0] raddr;
  output [(WDATA-1):0] rdata;

//local
  reg [(WDATA-1):0] mem[0:((1<<WADDR)-1)];
  reg [(WADDR-1):0] qraddr;

  always @(posedge clk) begin
    qraddr <= #1 raddr;
    if (we)
      mem[waddr] <= #1 wdata;
  end

  assign rdata = mem[qraddr];

endmodule

module clk_reset_gen(clk, reset);
  output clk;
  output reset;
  reg clk;
  reg reset;
  initial begin
    reset = 1;
    #5;
    clk = 0;
    #5;
    clk = 1;
    #5;
    reset = 0;
    clk = 0;
    forever #5 clk = ~clk;
  end
endmodule
