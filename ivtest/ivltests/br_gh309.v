module test();

`define TEST_MACRO(arg) arg

reg [3:0] value;

initial begin
  value = `TEST_MACRO({2'b01, 2'b10});
  if (value === 4'b0110)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
