module bug();

wire [2:0] Value1 = 0;

generate
  genvar i;

  for (i = 0; i < 8; i = i + 1) begin:Block
    wire [2:0] Value2;

    assign Value2 = Value1 + 7 - i;
  end
endgenerate

initial begin
  #1;
  $display("Block 0 value = %0d", Block[0].Value2);
  $display("Block 1 value = %0d", Block[1].Value2);
  $display("Block 2 value = %0d", Block[2].Value2);
  $display("Block 3 value = %0d", Block[3].Value2);
  $display("Block 4 value = %0d", Block[4].Value2);
  $display("Block 5 value = %0d", Block[5].Value2);
  $display("Block 6 value = %0d", Block[6].Value2);
  $display("Block 7 value = %0d", Block[7].Value2);
end

endmodule
