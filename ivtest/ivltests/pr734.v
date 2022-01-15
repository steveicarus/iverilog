/*
 * Test expressions with very wide reg variables.
 */

module test;
    parameter idx = 3584;       // Anything lower works
    reg [69119:0] mem;
    reg r;
    initial begin
        mem[idx] = 1;
        r = mem >> idx;
        if (r !== 1)
            $display("FAILED r = %b", r);
        else
            $display("PASSED");
    end
endmodule
