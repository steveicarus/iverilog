// Regression test for GitHub issue #62 : assert on invalid verilog input

module test();

reg [15:0] memory[3:0];

reg [15:0] vector;

reg [3:0] value;

initial begin
  value = memory[0][0];
  value = memory[0][0][3:0];
  value = memory[0][0][0 +: 4];
  value = memory[0][0][4 -: 4];
  value = vector[0][0];
  value = vector[0][3:0];
  value = vector[0][0 +: 4];
  value = vector[0][4 -: 4];
end

endmodule
