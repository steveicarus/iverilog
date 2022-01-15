`timescale 1 ps / 1 ps

module tester;
    wire  clko1, clko2;
    reg   clk;

    assign #50 clko1 = clk & 1'b1;
    assign #50 clko2 = clk;

    initial       clk = 1'b0;
    always  #5000 clk = ~ clk;

    initial $monitor( "%T %b %b %b", $time, clk, clko1, clko2 );
    initial #50001 $finish(0);
endmodule
