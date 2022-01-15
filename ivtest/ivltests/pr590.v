module ex1
(
clk,
reset,
insig,
outsig
);
input clk;
input reset;
input [3:0] insig;
output [3:0] outsig;
reg [3:0] outsig;

//reg [3:0] val_q;

always @ ( insig ) begin
        outsig = ~(4'hf << insig);
        $display("out: %b, in: %b",outsig,insig);
end

endmodule
module main;
reg             clk;
reg             reset;
reg     [3:0]   insig;
wire    [3:0]   outsig;
ex1 ex1(
 .clk(clk),
 .reset(reset),
 .insig(insig),
 .outsig(outsig));

initial
 begin
   $display ("\n starting the testbench\n");
   // set the inital value to 0
   clk        = 1'b0;
   reset  = 1;
   insig = 4'h0;
   #20 insig = 4'h2;
end
initial
  #71 $finish(0);

always
   #10 clk = ~clk;

always @(posedge clk)
  $display ($time, "..................clock tickling");

endmodule
