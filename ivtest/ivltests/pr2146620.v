module bug();

integer i;
integer j;

reg [7:0] Value;

initial begin
  for (i = 0; i < 2; i = i + 1) begin
    for (j = 0; j < 2; j = j + 1) begin
      Value = i * 256 + j * 128;
      $display(Value);
    end
  end
end

endmodule
