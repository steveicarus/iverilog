// This test was written to catch spurious "internal error" messages generated
// by the compiler, so uses a gold file for checking.
module test();

reg [7:0] Reg[0:3];

reg [7:0] Val0 = 255;
reg [7:0] Val1 = 1;
reg [7:0] Val2 = 2;
reg [7:0] Val3 = 3;

always @* begin
  Reg[0] = Val0;
  Reg[1] = Val1;
  Reg[2] = Val2;
  Reg[3] = Val3;
end

initial begin
  Val0 <= 0; // To make sure this triggers at T0 for SystemVerilog
  #1 $display("%0d %0d %0d %0d", Reg[0], Reg[1], Reg[2], Reg[3]);
  Val0 = 4;
  #1 $display("%0d %0d %0d %0d", Reg[0], Reg[1], Reg[2], Reg[3]);
  Val1 = 5;
  #1 $display("%0d %0d %0d %0d", Reg[0], Reg[1], Reg[2], Reg[3]);
  Val2 = 6;
  #1 $display("%0d %0d %0d %0d", Reg[0], Reg[1], Reg[2], Reg[3]);
  Val3 = 7;
  #1 $display("%0d %0d %0d %0d", Reg[0], Reg[1], Reg[2], Reg[3]);
end

endmodule
