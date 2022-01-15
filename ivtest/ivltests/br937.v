`begin_keywords "1364-2005"
module top;

localparam [8*8:0] expect = "0123456789";

reg [8*8:0] str;
reg passed;

initial begin
  passed = $value$plusargs("string=%s", str);
  $display("%h", str);
  if (passed && (str == expect))
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
`end_keywords
