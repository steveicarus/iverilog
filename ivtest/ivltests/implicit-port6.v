// test that if the signal doesn't exist, an error is thrown
module m(input a, output b);
assign b = a;
endmodule

module top;
reg a;
// wire b;
m foo(.*);
endmodule
