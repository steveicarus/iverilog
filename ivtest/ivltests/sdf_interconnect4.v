`timescale 1ns/1ps

/*
    This design tests the interconnection delay
    for three buffers in parallel with input and output vectors
*/

module buffer (
    input in,
    output out
);
    specify
        (in => out) = (0.0:0.0:0.0);
    endspecify

    assign out = in;

endmodule

module my_design (
    input [2:0] a,
    output [2:0] b
);

    buffer buffer0 (
        .in (a[0]),
        .out (b[0])
    );

    buffer buffer1 (
        .in (a[1]),
        .out (b[1])
    );

    buffer buffer2 (
        .in (a[2]),
        .out (b[2])
    );

endmodule

module top;

    reg [2:0] a;
    wire [2:0] b;

    initial begin
        $sdf_annotate("ivltests/sdf_interconnect4.sdf", my_design_inst);
        $monitor("time=%0t a=%b b=%b", $realtime, a, b);
    end

    initial begin
        #5;
        a <= 3'b000;
        #10;
        a <= 3'b111;
        #10;
        $finish;
    end

    my_design my_design_inst (
        .a (a),
        .b (b)
    );

endmodule
