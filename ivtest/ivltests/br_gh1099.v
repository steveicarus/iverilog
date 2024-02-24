module top;
    reg [3:0] inp1;
    reg signed [3:0] inp2;
    wire [4:0] out1, out2;
    initial begin
        $monitor("%b %b %b %b", inp1, inp2, out1, out2);
        #1 inp1 = 4'b1111;
        #1 inp2 = 4'b1111;
        #1;
        if ((out1 === 5'b01111) && (out2 === 5'b01111))
            $display("PASSED");
        else
            $display("FAILED");
    end
    mod m1({inp1}, out1);
    mod m2({inp2}, out2);
endmodule

module mod(
    input [4:0] inp,
    output [4:0] out
);
    assign out = inp;
endmodule
