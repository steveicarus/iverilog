module bug;

wire [7:0] r1;
wire [7:0] r2;
wire [7:0] r3;
wire [7:0] r4;
wire [7:0] r5;
wire [7:0] r6;
wire [7:0] r7;

wire [7:0] r;

function [7:0] fn;

input [7:0] a;
input [7:0] b;
input [7:0] c;
input [7:0] d;
input [7:0] e;
input [7:0] f;
input [7:0] g;
input [7:0] h;

begin
  fn = a + b + c + d + e + f + g + h;
end

endfunction

assign {r1, r2, r3, r4, r5, r6, r7} = 56'd257;

assign r = fn(r1, r2, r3, r4, r5, r6, r7, 8'd0);

initial begin
   #1 $display("r=%0d", r);
   if (r !== 8'd2) begin
      $display("FAILED");
      $finish;
   end
   $display("PASSED");
end

endmodule
