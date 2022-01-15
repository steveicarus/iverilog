`timescale 1 ps / 1 ps

module tester;
    wire  clko1, clko2;
    reg   clk1,  clk2, f1, f2;

    ckmux uut ( clko1, clk1, clk2, f1, f2 );

    assign #50 clko2 = clk1;

    initial begin
        f1 = 1'b0;
        f2 = 1'b1;
    end

    initial begin
        clk1 = 1'b0;
        forever #5000 clk1 = ~ clk1;
    end

    initial begin
        clk2 = 1'b0;
        forever #5100 clk2 = ~ clk2;
    end

    initial $monitor( "%T %b %b %b", $time, clk1, clko1, clko2 );
    initial #50001 $finish(0);
endmodule

module ckmux ( clko1, clk1, clk2, f1, f2 );
    output    clko1;
    input     clk1, clk2, f1, f2;

    reg       dclk1ff;
    wire      dclk1;

    initial begin
        dclk1ff = 1'b0;
        forever @( negedge clk1 ) dclk1ff <= #50 ~ dclk1ff;
    end

    assign #50 dclk1 = f2 ? dclk1ff : clk2;
    assign #50 clko1 = f1 ? dclk1   : clk1;
endmodule
