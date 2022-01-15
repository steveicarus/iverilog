module tb;

   reg [1:0] i;
   reg [3:0] x[0:2];

   initial begin
      x[1] = 1;
      i = 1;
      x[i++] += 2;
      if (x[1] != 3) $display("FAILED: got %b", x[1]);
      else $display("PASSED");
   end
endmodule // tb
