// specify4.v

module top;
   reg d, c;
   wire q;

   initial begin
      d = 0;
      c = 1;
      #10 $monitor($time,,"q=%b, d=%b, c=%b", q, d, c);
      #5 c = 0;
      #5 c = 1;
      #5 c = 0; d = 1;
      #5 c = 1;
      #5 c = 0; d = 0;
      #5 c = 1;
      #10 $finish(0);
   end

   mydff g1 (q, d, c);

endmodule

module mydff (output reg q, input d, input c);

   always @(posedge c) q <= d;
   specify
      (posedge c => (q +: d)) = (3, 2);
   endspecify
endmodule
