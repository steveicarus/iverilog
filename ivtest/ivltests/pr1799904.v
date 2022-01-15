module test;


reg clock;
initial begin
clock = 0;
forever #5 clock = !clock;
end

wire [0:31] read_data1 [0:7];
reg [0:31] read_data2 [0:7];


assign read_data1[0] = 0;
assign read_data1[1] = 1;
assign read_data1[2] = 2;
assign read_data1[3] = 3;
assign read_data1[4] = 4;
assign read_data1[5] = 5;
assign read_data1[6] = 6;
assign read_data1[7] = 7;


always @(posedge clock) begin: we
reg [3:0] x;
for (x=0; x<8; x=x+1) begin
read_data2[x[2:0]] <= read_data1[x[2:0]];
end
end

always @(posedge clock) begin: wg
integer i;
#1 for (i=0; i<8; i=i+1) begin
$write("%x ", read_data2[i]);
end
$display;
end

initial begin
#20;
$finish(0);
end


endmodule // test
