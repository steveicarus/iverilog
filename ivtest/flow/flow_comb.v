// Richer Phase 2 test: comb always with case, ternary, continuous
// assign, and two levels of hierarchy.

module alu (
    input  wire [1:0] op,
    input  wire [7:0] a,
    input  wire [7:0] b,
    output reg  [7:0] y
);
    always @(*) begin
        case (op)
            2'b00: y = a + b;
            2'b01: y = a - b;
            2'b10: y = a & b;
            default: y = a ^ b;
        endcase
    end
endmodule

module datapath (
    input  wire [1:0] sel,
    input  wire [7:0] x,
    input  wire [7:0] z,
    output wire [7:0] result,
    output wire       is_zero
);
    wire [7:0] alu_out;
    alu u_alu (.op(sel), .a(x), .b(z), .y(alu_out));
    assign result  = alu_out;
    assign is_zero = (alu_out == 8'b0) ? 1'b1 : 1'b0;
endmodule

module chip (
    input  wire [1:0] mode,
    input  wire [7:0] in0,
    input  wire [7:0] in1,
    output wire [7:0] out,
    output wire       zero
);
    datapath u_dp (
        .sel    (mode),
        .x      (in0),
        .z      (in1),
        .result (out),
        .is_zero(zero)
    );
endmodule
