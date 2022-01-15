/*
 * This is based on PR#1029
 */
module main();

`define none
`define fred eric
`define bill main`none.eric
reg [8*8:0] eric;
initial
  begin
    eric = "PASSED";
    $display("%0s",`bill);
    $finish ;
  end

endmodule
