module pr2974294;

reg  [7:0] array[1:0];
wire [7:0] word;
reg        fail;

assign word = array[0];

initial begin
  fail = 0;
  #0 $display("%b", word);
  if (word !== 8'bx) fail = 1;
  #1 $display("%b", word);
  if (word !== 8'bx) fail = 1;
  array[0] = 8'd0;
  #0 $display("%b", word);
  if (word !== 8'd0) fail = 1;
  if (fail)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
