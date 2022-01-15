module test();

wire [15:0] data = 8;
wire [15:0] result;

assign result = data + $ivlh_to_unsigned(8, 16);

initial begin
  #1 $display("result = %0d", result);
  if (result === 16)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
