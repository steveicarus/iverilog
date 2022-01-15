module main();
  parameter INIT_00 = 32'hffffffff;
  reg [17:0] t;
  reg [8:0] c;
  reg error ;
  initial begin
    error = 0;
    c = 0;
    $display("%b",INIT_00[c]);
    c = 1;
    $display("%b",INIT_00[c]);

    t = {17'd0,INIT_00[0]}<<1;
    if(t !== 17'b0_0000_0000_0000_0010)
      begin
         $display("FAILED - shift operation {17'd0,INIT_00[0]}<<1; %b",t);
	 error = 1;
      end
    else
      $display("%b",t);
    c = 0;
    t = {17'd0,INIT_00[c]}<<1;
    if(t !== 17'b0_0000_0000_0000_0010)
      begin
         $display("FAILED - shift operation {17'd0,INIT_00[c]}<<1 %b",t);
	 error = 1;
      end
    else
      $display("%b",t);
    c = 16;
    t = {17'd0,INIT_00[c]}<<1;
    if(t !== 17'b0_0000_0000_0000_0010)
      begin
         $display("FAILED - shift operation {17'd0,INIT_00[c]}<<1 %b",t);
	 error = 1;
      end
    else
      $display("%b",t);

    if(error == 1)
         $display("FAILED");
    else
         $display("PASSED");
  end
endmodule
