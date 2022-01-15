module test;

parameter DATA_DEPTH_A = 8;
parameter DATA_DEPTH_B = 2**8;
reg [(2**8)-1:0] data1 ; //this gets compiled
reg [7:0] data2 [DATA_DEPTH_A-1:0]; // this gets compiled
reg [DATA_DEPTH_A-1:0] data3 [7:0]; // this gets compiled
reg [DATA_DEPTH_B-1:0] data4 [7:0]; // this gets compiled
reg [7:0] data5 [(2**8)-1:0]; // results in compilation error
reg [7:0] data6 [DATA_DEPTH_B-1:0]; // results in compilation error

endmodule
