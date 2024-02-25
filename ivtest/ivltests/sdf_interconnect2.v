`timescale 1ns/1ps

/*
    This design tests the interconnection delay
    for three buffers in parallel
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
    input a,
    output b
);
    wire w1, w2, w3;

    buffer buffer0 (
        .in (a),
        .out (w1)
    );

    buffer buffer1 (
        .in (a),
        .out (w2)
    );

    buffer buffer2 (
        .in (a),
        .out (w3)
    );

    assign b = w1 & w2 & w3;

endmodule

module top;

    reg a;
    wire b;

    initial begin
        $sdf_annotate("ivltests/sdf_interconnect2.sdf", my_design_inst);
        $monitor("time=%0t a=%h b=%h", $realtime, a, b);
    end

    initial begin
        #5;
        a <= 1'b0;
        #10;
        a <= 1'b1;
        #10;
        $finish;
    end

    my_design my_design_inst (
        .a (a),
        .b (b)
    );

endmodule
