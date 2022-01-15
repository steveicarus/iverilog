 module RegisterArrayBug01;

 reg [15:0] rf[0:7];

 wire [15:0] rf_0 = rf[1];

 initial begin
 $monitor($time,, "rf[0] is %h %h", rf[1], rf_0);

 rf[1] = 16'hffff;
 #10 rf[1] = 16'h0000;
 #10 rf[1] = 16'hbeef;
 #10 $finish(0);
 end

 endmodule

 /*
 System prints:
 0 rf[0] is xxxx ffff
 10 rf[0] is xxxx 0000
 20 rf[0] is beef beef
 Expected is:
 0 rf[0] is ffff ffff
 10 rf[0] is 0000 0000
 20 rf[0] is beef beef
 */
