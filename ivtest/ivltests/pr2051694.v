module test_unknown_in_always_star;
reg [2:0] x;
always @* x = {unknown, 1'b0};
endmodule
