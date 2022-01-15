module t;
reg [7:0] x = 1;

initial #5 x = 2;
always @(x) #5 x = 3;

final begin
   if (x == 3) $display("x =%d, PASSED", x);
   $finish(0);
   $display("FAILED! Executed past $finish in final block!");
end
endmodule

module t2;
final $display("t2 final");
endmodule
