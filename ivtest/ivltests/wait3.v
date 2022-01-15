/*
 * this test attempts to show a problem with the waits. This skip
 * and skip2 modules should have identical behavior.
 */

module skip(r,a);
input r;
output a;
wire r;
reg a;
initial
   a=0;
always begin
   wait(r);
   #1 a=1;
   wait(!r);
   #1 a=0;
end
endmodule

module skip2(r,a);
input r;
output a;
wire r;
reg a;
initial
   a=0;
always @ (r or a) begin
   case ({r,a})
   00: ; // idle
   10: #1 a=1;
   11: ; // idle
   01: #1 a=0;
   endcase
end
endmodule

module test;
reg r1;
wire a1;
reg clk;
// skip2 skip1(r1,a1); // simulates as expected
skip skip1(r1,a1); // simulation hangs

always #50 clk= !clk;

initial begin
   $monitor($time," ",r1,a1);
   $display("starting");
   #100 r1=0;
   #100 r1=1;
   wait(a1);
   #100 r1=0;
   #1000 $display("timeout"); $finish(0);
end
endmodule
