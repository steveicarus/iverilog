module br918b();

wire [3:0] w;

assign (pull1,strong0) w[1:0] = 2'b00;
assign (pull1,strong0) w[1:0] = 2'b10;

assign (pull1,strong0) w[3:2] = 2'b11;
assign (pull1,strong0) w[3:2] = 2'b10;

initial begin
  #1 $display("%b", w);
  if (w === 4'b1000)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
