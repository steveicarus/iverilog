module test;
wire [3:0] a;
reg [1:0] b;
assign a[0+:2] = b;
assign a[3-:2] = b;

initial begin
   b = 2'b01;
   #1 if (a !== 4'b0101) begin
      $display("FAILED -- b=%b, a=%b", b, a);
      $finish;
   end

   b=2'b10;
   #1 if (a !== 4'b1010) begin
      $display("FAILED -- b=%b, a=%b", b, a);
      $finish;
   end

   $display("PASSED");
end
endmodule
