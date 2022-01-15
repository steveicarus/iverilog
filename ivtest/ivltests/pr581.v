module module1(clock,reset,result);
input clock,reset;
output result;
reg result;
always @ (posedge clock)
if (reset) result <= 0; else result <= 1;
endmodule

// driver
module main;
   reg clk,reset;
   reg data[1:3]; // ILLEGAL port connection NOT detected
// to fix, use wire data_1,data_2,data_3;
   module1 inst1(clk,reset,data[1]);
   module1 inst2(clk,reset,data[2]);
   module1 inst3(clk,reset,data[3]);

always #50 clk = ~clk;
initial begin

$monitor($time,"clk=%b,reset=%b,%b%b%b",clk,reset,data[1],data[2],data
[3]);
   clk = 0;
   reset = 1;
   #200 reset = 0;
   #200 $display("driver timeout"); $finish;
end
endmodule
