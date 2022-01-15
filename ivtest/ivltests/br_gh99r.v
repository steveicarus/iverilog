module test();

wire [7:0] value1;
wire [9:0] value2;

assign value1[3:0] = 4'd2;

assign value2 = $signed(value1);

initial begin
  #2 $display("%b %b", value1, value2);
  if (value2 === 10'bzzzzzz0010)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
