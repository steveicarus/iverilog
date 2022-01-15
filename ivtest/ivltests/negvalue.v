module negvalue;

 reg[7:0]reg1;

 initial begin
  reg1 <=   -13 +21 ;
  #1
  reg1 <= 0 -13 +21 ;
 end

 always@(reg1)begin
  $display("%d (should be 8)",reg1);
 end

endmodule
