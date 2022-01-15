`begin_keywords "1364-2005"
module top(arg);
    input [31:0] arg;

    wire [31:0] out_0;
    wire [31:0] out_1;
    reg  [31:0] var;

    add dut_0 (var, var, out_0);
    add dut_1 (arg, var, out_1);

endmodule

module add(in0, in1, out);
    input      [31:0]  in0;
    input      [31:0]  in1;
    output reg [31:0]  out;

    // This works if you explicitly specify the sensitivity list.
    always @* out = in0 + in1;
endmodule
`end_keywords
