`timescale 1ns/1ps

module fail ();
   reg pz;
   reg [4:0] p;
   wire      em, net0102;

   initial begin
      p = 0;
      pz = 0;

      $monitor("time=%0t", $time,": em=", em, ", pz=", pz, ", p=%b", p);

      while (p < 5'b11111)
	#10 p = p+1;

      #1; // avoid final race
      $finish(0);
   end

   nr1 I1 (em, net0102, pz, p[0]);
   nr2 I2 (net0102, p[1], p[2], p[3], p[4]);
endmodule

module nr1 (zn, a1, a2, a3);
  output zn;
  input a1, a2, a3;

  not G1(N1, a1);
  not G2(zn, N2);
  or  G3(N2, N1, a2, a3);

  specify
    (a1 +=> zn) = (0.500, 0.500);
    (a2 -=> zn) = (0.500, 0.500);
    (a3 -=> zn) = (0.500, 0.500);
  endspecify
endmodule

module nr2 (zn, a1, a2, a3, a4);
  output zn;
  input a1, a2, a3, a4;

  or  G1(N1, a1, a2, a3, a4);
  not G2(zn, N1);

  specify
    (a1 -=> zn) = (0.500, 0.500);
    (a2 -=> zn) = (0.500, 0.500);
    (a3 -=> zn) = (0.500, 0.500);
    (a4 -=> zn) = (0.500, 0.500);
  endspecify
endmodule
