module top;

wire [2:0] value = 2;

shim shim(
  .bit0(value[0]),
  .bit1(value[1]),
  .bit2(value[2])
);

endmodule

module shim(
  inout wire bit0,
  inout wire bit1,
  inout wire bit2
);

wire [2:0] value = {bit2, bit1, bit0};

initial begin
  #1 $display(value);
  if (value === 2)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
