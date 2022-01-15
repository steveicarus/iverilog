// PR1845683
/**
* Author: Evan Lavelle
* Company: Riverside Machines Ltd.
* Date: 06/12/2007
*
* Cver feature #1; signed arithmetic
*
* The correct output should be:
*
* res1: '00101010'; res2: '00101010'; res3: '00101010'
*
* Cver reports:
*
* res1: '10101010'; res2: '10101010'; res3: '10101010'
*/
module test;
reg [7:0] res1, res2;
reg signed [7:0] res3;
initial
begin
res1 = 8'sb11001100 ^ 7'sb1100110;
res2 = $signed(8'b11001100) ^ $signed(7'b1100110);
res3 = $signed(8'b11001100) ^ $signed(7'b1100110);
$display("res1: '%b'; res2: '%b'; res3: '%b'", res1, res2, res3);
end
endmodule
