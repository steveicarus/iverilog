`timescale 1ns/1ps

/*
    This design tests the interconnection delays
    for a circuit of various buffers and xors
*/

module my_xor (
    input a,
    input b,
    output out
);
    specify
        (a => out) = (0.0:0.0:0.0);
        (b => out) = (0.0:0.0:0.0);
    endspecify

    assign out = a ^ b;

endmodule

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
    input a,
    input b,
    input c,
    output d
);
    wire w1, w2, w3, w4, w5, w6, w7;

    buffer buffer0 (
        .in (a),
        .out (w1)
    );

    my_xor my_xor0 (
        .a (b),
        .b (c),
        .out (w2)
    );

    my_xor my_xor1 (
        .a (w1),
        .b (b),
        .out (w3)
    );

    buffer buffer1 (
        .in (w2),
        .out (w4)
    );

    my_xor my_xor2 (
        .a (w3),
        .b (w4),
        .out (w5)
    );

    buffer buffer2 (
        .in (c),
        .out (w6)
    );

    my_xor my_xor3 (
        .a (w5),
        .b (w6),
        .out (w7)
    );

    buffer buffer3 (
        .in (w7),
        .out (d)
    );

endmodule

module top;

    reg a, b, c;
    wire d;

    initial begin
        $sdf_annotate("ivltests/sdf_interconnect3.sdf", my_design_inst);
        $monitor("time=%0t a=%h b=%h c=%h d=%h", $realtime, a, b, c, d);
    end

    initial begin
        #10;
        a <= 1'b0;
        b <= 1'b0;
        c <= 1'b0;
        #10;
        a <= 1'b1;
        b <= 1'b0;
        c <= 1'b0;
        #10;
        a <= 1'b0;
        b <= 1'b1;
        c <= 1'b0;
        #10;
        a <= 1'b1;
        b <= 1'b1;
        c <= 1'b0;
        #10;
        a <= 1'b0;
        b <= 1'b0;
        c <= 1'b1;
        #10;
        a <= 1'b1;
        b <= 1'b0;
        c <= 1'b1;
        #10;
        a <= 1'b0;
        b <= 1'b1;
        c <= 1'b1;
        #10;
        a <= 1'b1;
        b <= 1'b1;
        c <= 1'b1;
        #10;
        $finish;
    end

    my_design my_design_inst (
        .a (a),
        .b (b),
        .c (c),
        .d (d)
    );

endmodule
