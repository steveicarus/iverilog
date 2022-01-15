/*
 * 1364-2001 19.3.2 "An undefined text macro has no value, just as if it had
 * never been defined."
 */
`define a 1

`ifdef a
`define b 1
`else
`define b 0
`endif

`undef a

`ifdef a
`define c 1
`else
`define c 0
`endif

module test;
initial begin
    if(`a+1 !== 1) begin $display("FAIL"); $finish; end
    if(`b+1 === 1) begin $display("FAIL"); $finish; end
    if(`c+1 !== 1) begin $display("FAIL"); $finish; end

    $display("PASSED");
end
endmodule
