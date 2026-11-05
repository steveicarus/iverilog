// Phase 0 test design: a counter instantiated inside a top module,
// to exercise scope hierarchy + nexus reachability in tgt-flow.

module counter #(parameter WIDTH = 4) (
    input  wire             clk,
    input  wire             rst,
    input  wire             en,
    output reg  [WIDTH-1:0] count
);
    always @(posedge clk or posedge rst) begin
        if (rst)
            count <= {WIDTH{1'b0}};
        else if (en)
            count <= count + 1'b1;
    end
endmodule

module top (
    input  wire       clk,
    input  wire       rst,
    output wire [3:0] q
);
    wire enable = 1'b1;
    counter #(.WIDTH(4)) u_cnt (
        .clk   (clk),
        .rst   (rst),
        .en    (enable),
        .count (q)
    );
endmodule
