// Strictly speaking this is illegal as it uses a hierarchical name in a
// constant expression.
module top;
    parameter ENABLE = 1;
    if (ENABLE) begin : blk
        wire [7:0] w;
    end
    wire [7:0] x;
    wire [$bits(blk.w)-1:0] y = 8'h55;
    wire [$bits(x)-1:0] z = 8'haa;
    initial begin
        $display("blk.w: %b (%0d bits)", blk.w, $bits(blk.w));
        $display("x: %b (%0d bits)", x, $bits(x));
        $display("y: %b (%0d bits)", y, $bits(y));
        $display("z: %b (%0d bits)", z, $bits(z));
        if (y === 8'h55 && z === 8'haa)
            $display("PASSED");
        else
            $display("FAILED");
    end
endmodule
