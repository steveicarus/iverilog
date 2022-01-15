module bug();

reg [3:0] flags = 4'b0000;

generate
  genvar i;

  for (i = 1; i < 4; i = i + 1) begin:loop
    localparam j = i;

    if (j > 0) begin
      initial #1 flags[j] = 1'b1;
    end
  end
endgenerate

initial begin
  #2 $display("flags = %b", flags);
  if (flags === 4'b1110)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
