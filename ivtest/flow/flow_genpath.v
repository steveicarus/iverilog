// Generate frame with WHOLE-signal port connections (isolates frame
// transparency from bit-select handling).
module inv8 (input wire [7:0] d, output wire [7:0] q);
    assign q = ~d;
endmodule

module wrap (input wire [7:0] din, output wire [7:0] dout);
    generate
        if (1) begin : blk
            inv8 u_inv (.d(din), .q(dout));
        end
    endgenerate
endmodule
