`define check(expr) \
    result = expr; \
    $display("%b", result); \
    if (result !== 32'b11111111111111111111111111011111) failed = 1;

module top;
    integer x;
    localparam Y = 1'sb1;
    localparam integer Z = 1'sb1;
    reg [31:0] result;
    reg failed = 0;
    initial begin
        `check($bits(x) ^ 1'sb1);
        `check($bits(x) ^ Y);
        `check($bits(x) ^ Z);
        `check($signed($bits(x)) ^ 1'sb1);
        `check($signed($bits(x)) ^ Y);
        `check($signed($bits(x)) ^ Z);
         if (failed)
             $display("FAILED");
         else
             $display("PASSED");
    end
endmodule
