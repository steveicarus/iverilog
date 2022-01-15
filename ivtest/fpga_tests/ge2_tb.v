module main;

   wire out;
   reg [1:0] A, B;

   ge2 dut(.out(out), .A(A), .B(B));

   initial begin
      A = 0;
      B = 0;
      #1 $display("%b >= %b: %b", A, B, out);

      B = 1;
      #1 $display("%b >= %b: %b", A, B, out);

      B = 2;
      #1 $display("%b >= %b: %b", A, B, out);

      B = 3;
      #1 $display("%b >= %b: %b", A, B, out);

      A = 1;
      B = 0;
      #1 $display("%b >= %b: %b", A, B, out);

      B = 1;
      #1 $display("%b >= %b: %b", A, B, out);

      B = 2;
      #1 $display("%b >= %b: %b", A, B, out);

      B = 3;
      #1 $display("%b >= %b: %b", A, B, out);

      A = 2;
      B = 0;
      #1 $display("%b >= %b: %b", A, B, out);

      B = 1;
      #1 $display("%b >= %b: %b", A, B, out);

      B = 2;
      #1 $display("%b >= %b: %b", A, B, out);

      B = 3;
      #1 $display("%b >= %b: %b", A, B, out);

      A = 3;
      B = 0;
      #1 $display("%b >= %b: %b", A, B, out);

      B = 1;
      #1 $display("%b >= %b: %b", A, B, out);

      B = 2;
      #1 $display("%b >= %b: %b", A, B, out);

      B = 3;
      #1 $display("%b >= %b: %b", A, B, out);

   end // initial begin
endmodule // main
