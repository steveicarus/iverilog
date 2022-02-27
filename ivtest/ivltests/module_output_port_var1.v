// Check that ANSI output ports that have a Verilog data type are elaborated as
// variables and be assigned a value.

module test (
  output reg a,
  output reg [1:0] b,
  output reg signed [1:0] c,
  output integer d,
  output time e
);

initial begin
  a = 0;
  b = 0;
  c = 0;
  d = 0;
  e = 0;
  $display("PASSED");
end

endmodule
