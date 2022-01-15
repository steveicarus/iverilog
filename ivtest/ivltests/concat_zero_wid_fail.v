module top;
  reg [7:0] result;

  initial begin
    result = {{0{1'b1}}}; // This fails concatination has zero width.
  end
endmodule
