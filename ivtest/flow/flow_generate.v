// Phase 3 test: for-generate replicating a sub-module, plus an
// if-generate, to exercise generate-frame emission.

module inv #(parameter ID = 0) (
    input  wire d,
    output wire q
);
    assign q = ~d;
endmodule

module gen_top #(parameter N = 3) (
    input  wire [2:0] din,
    output wire [2:0] dout
);
    genvar i;
    generate
        for (i = 0; i < 3; i = i + 1) begin : row
            inv #(.ID(i)) u_cell (.d(din[i]), .q(dout[i]));
        end
    endgenerate

    generate
        if (N > 0) begin : opt
            wire tap;
            assign tap = din[0];
        end
    endgenerate
endmodule
