module foo(input x);
parameter n = 0;
pulldown p1(x);
initial #n $display("x(%0d) : %b", n, x);
endmodule

module tb;
wire y;
wire z;
foo #1 bar1(1'b0);
foo #2 bar2(1'b1);
foo #3 bar3(1'bz);
foo #4 bar4(y);
foo #5 bar5({z});
initial #6 $display("y    : ", y);
initial #7 $display("z    : ", z);
endmodule
