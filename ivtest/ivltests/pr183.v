// Sample Code
module main( stb, a );

   input        stb;
   output [1:0] a;
   wire   [1:0] b;

   buf (a[0], b[0]);
   buf (a[1], b[1]);

   specify
      (posedge stb => (a[0]:1'bx)) = 1.0;
      (posedge stb => (a[1]:1'bx)) = 1.0;
   endspecify

endmodule // main
