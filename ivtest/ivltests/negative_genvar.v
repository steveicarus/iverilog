module negative_genvar;

wire signed [3:0] value[-7:7];

genvar i;

for (i = 7; i >= -7; i = i - 1) begin:genloop
  assign value[i] = i;
end

integer j;

reg fail = 0;

initial begin
  #0;
  for (j = -7; j <= 7; j = j + 1) begin
    $display("%d", value[j]);
    if (value[j] !== j) fail = 1;
  end

  if (fail)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
