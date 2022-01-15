module br918a();

reg [1:0] v1;
reg [1:0] v2;
reg [1:0] v3;
reg [1:0] v4;

wire [3:0] w;

assign (pull1,strong0) w[1:0] = v1;
assign (pull1,strong0) w[1:0] = v2;

assign (pull1,strong0) w[3:2] = v3;
assign (pull1,strong0) w[3:2] = v4;

initial begin
  v1 = 2'b00;
  v2 = 2'b10;
  v3 = 2'b11;
  v4 = 2'b10;
  #1 $display("%b", w);
  if (w === 4'b1000)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
