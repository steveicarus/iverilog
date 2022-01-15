module test;

    reg [63:0] out;
    reg [5:0] in;

    integer i = 0;

    reg failed = 0;

    initial begin
        for (i = 0; i < 64; i = i + 1) begin
            in = i;
            out = 2 ** in;
            $display("%d: %b", i, out);
            if (out !== 64'd1 << i)
                failed = 1;
        end
        if (failed)
            $display("FAILED");
        else
            $display("PASSED");
    end
endmodule
