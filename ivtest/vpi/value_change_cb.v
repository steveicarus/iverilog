`timescale 1s/1ms

module test;

logic [3:0] v4;
wire  [3:0] w4;
integer     i4;

bit   [3:0] v2;
byte        b2;
shortint    s2;
int         i2;
longint     l2;

real        r;

event       e;

logic [3:0] p4;

logic [3:0] a4[3:0];
bit   [3:0] a2[3:0];

assign w4 = v4;

initial begin
    $my_monitor(v4, w4, i4, v2, b2, s2, i2, l2, r, e, p4[1:0], a4, a2[1]);
    #1 v4 = 4'd1;
    #1 i4 = 2;
    #1 v2 = 4'd3;
    #1 b2 = 4;
    #1 s2 = 5;
    #1 i2 = 6;
    #1 l2 = 7;
    #1 r  = 8.0;
    #1 ->e;
    // NOTE: the value change callback on a part select returns the value of the entire variable.
    #1 p4 = 4'd10;
    #1 a4[0] = 4'd11;
    #1 a4[1] = 4'd12;
    #1 a2[0] = 4'd13;
    #1 a2[1] = 4'd14;
end

endmodule
