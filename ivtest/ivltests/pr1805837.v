module test();
  wire [2:0] var1 = 3'bx01; // Incorrect results 101 vs x01.
  wire [2:0] var2 = 3'bx10; // Incorrect results 010 vs x10.
  wire [2:0] var3 = 3'bx0z; // Incorrect results zzz vs x0z.
  wire [2:0] var4 = 3'bx1z; // Incorrect results zzz vs x1z.
  wire [2:0] var5 = 3'bxz1; // Incorrect results 1z1 vs xz1.
  wire [2:0] var6 = 3'bxz0; // Incorrect results 0z0 vs xz0.
  wire [3:0] var7 = 4'bx0z0; // Incorrect results 0000 vs x0z0.
  wire [2:0] var8 = 3'bxxx; // This works correctly.

  initial begin
    $displayb("Should be:\nx01 x10 x0z x1z xz1 xz0 x0z0 xxx");
    $strobeb (var1,, var2,, var3,, var4,, var5,, var6,, var7,, var8);
  end
endmodule
