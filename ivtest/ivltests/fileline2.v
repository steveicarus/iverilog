/*
 * P1800/D8 22.13
 * "A `line directive changes `__LINE__, and may change `__FILE__ as well."
 */
module aaa;
reg pass;

`define printfl(x) $display("%0d -> %s:%0d", x, `__FILE__, `__LINE__)
initial begin #0
    pass = 1;

`line 1000 "./ivltests/fileline2.v" 0
    if(`__LINE__ !== 1000) begin
        $display("FAIL"); pass = 0;
    end
    `printfl(1);

`line 2000 "imaginary-include-file" 1
    if(`__LINE__ !== 2000) begin
        $display("FAIL"); pass = 0;
    end
    `printfl(2);

`line 3000 "./ivltests/fileline2.v" 2
    if(`__LINE__ !== 3000) begin
        $display("FAIL"); pass = 0;
    end
    `printfl(3);


    if(pass) $display("PASSED");
end
endmodule
