module tb();

reg [15:0] array[1:0];

reg [3:0] shift_distance;

wire [15:0] shifted_value;

assign shifted_value = array[0] >> shift_distance;

initial begin
  array[0] = 16'h1234;
  shift_distance = 4;
  #0;
  if (shifted_value === 16'h0123)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
