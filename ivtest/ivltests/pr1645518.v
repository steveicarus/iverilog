/* PR1645518 */

module testBench;
   wire w1, w2, w3, w4, w5;
   binaryToESeg d (w1, w2, w3, w4, w5);
   test_bToESeg t (w1, w2, w3, w4, w5);
endmodule

module binaryToESeg
  (input A, B, C, D,
   output eSeg);
   nand #1
     g1 (p1, C, ~D),
     g2 (p2, A, B),
     g3 (p3, ~B, ~D),
     g4 (p4, A, C),
     g5 (eSeg, p1, p2, p3, p4);
endmodule // binaryToESeg

module test_bToESeg
  (output reg A, B, C, D, input eSeg);
 initial // two slashes introduce a single line comment
   begin
      $monitor ($time,,
		"A = %b B = %b C = %b D = %b, eSeg = %b",
		A, B, C, D, eSeg);
      //waveform for simulating the nand lip lop
      #10 A = 0; B = 0; C = 0; D = 0;
      #10 D = 1;
      #10 C = 1; D = 0;
      #10 $finish(0);
   end
endmodule
