module pr3064375;

reg		CLK;
reg		RST;

reg		Reg1;
reg		Reg2;

initial begin
  CLK = 0;
  forever begin
    #5 CLK = 1;
    #5 CLK = 0;
  end
end

initial begin
  RST = 1;
  #20;
  RST = 0;
  #101;
  $finish(0);
end

always @(posedge CLK or posedge RST) begin
  if (RST)
    Reg1 <= 0;
  else
    Reg1 <= !Reg1;
end

always @(negedge CLK or posedge RST) begin
  if (RST)
    Reg2 <= 0;
  else
    Reg2 <= Reg1;
end

initial begin
  $monitor("CLK %b RST %b Reg1 %b Reg2 %b", CLK, RST, Reg1, Reg2);
end

endmodule
