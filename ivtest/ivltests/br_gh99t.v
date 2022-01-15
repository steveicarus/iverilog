module test();

wire [7:0] value1;
wire [7:0] value2;

assign (strong1,weak0) value1[3:0] = 4'b1010;

nmos buffer[7:0](value2, value1, 1'b1);

assign (strong1,weak0) value2 = 8'b00110011;

initial begin
  #2 $display("%b %b", value1, value2);
  if (value2 === 8'b00111011)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
