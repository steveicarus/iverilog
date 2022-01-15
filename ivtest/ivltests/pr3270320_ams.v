module bug();

wire [7:0] b;

assign b = $abs(missing);

endmodule
