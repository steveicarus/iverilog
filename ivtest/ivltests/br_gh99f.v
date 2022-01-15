module test();

wire [7:0] value1;
wire [7:0] value2;

assign value1[3:0] = 4'd2;

assign value2 = $itor(value1);

initial begin
  // 'z' bits are converted to '0'.
  #2 $display("%b %b", value1, value2);
  if (value2 === 8'b00000010)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
