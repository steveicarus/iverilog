// Check compiler can handle zero widths in indexed
// part selects.
module bug();

localparam off1 = 1;
localparam wid1 = 0;

integer off2 = 1;
integer wid2 = 0;

wire [7:0] vector = 8'h55;

wire part1 = |vector[off1 +: wid1];
wire part2 = |vector[off2 +: wid2];

endmodule
