`begin_keywords "1364-2005"
module signed_net_display;

wire signed [3:0] value[-7:7];

genvar i;

for (i = 7; i >= -7; i = i - 1) begin:genloop
  assign value[i] = i;
end

reg [2*8-1:0] result;
reg [2*8-1:0] expect;

integer j;

reg fail = 0;

initial begin
  #0;
  for (j = -7; j <= 7; j = j + 1) begin
    $swrite(result, "%d", value[j]);
    if (j < 0)
      begin
        expect[1*8 +: 8] = "-";
        expect[0*8 +: 8] = "0" - j;
      end
    else
      begin
        expect[1*8 +: 8] = " ";
        expect[0*8 +: 8] = "0" + j;
      end
    $display("%s : %s", expect, result);
    if (result !== expect) fail = 1;
  end

  if (fail)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
`end_keywords
