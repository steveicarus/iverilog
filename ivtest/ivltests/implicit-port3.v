// test that if the port doesn't exist, an error is thrown
module m(input a, output b);
assign b = a;
endmodule

module top;
reg a;
wire b;
wire c;
m foo(.a, .b, .c);
endmodule
