module test();
reg		c;

  a #(1) ua( .c(c), .b(h));

  initial begin
    c = 0;
    #1 c = 1;
    #1 c = 0;
    $display("PASSED");
  end

endmodule
module a(
		c,
		b,
		);
   parameter    e       = 2;

   input	     c;
   output [e-1:0]    b;

   reg  [e-1:0]      f;
   reg  [e-1:0]      g;
   reg  [e-1:0]      b;
   integer	     d;

   always @(posedge c) begin
     for(d=0; d<e; d=d+1)
       b[d] <= (f[d] & (b[d] | g[d]));
   end
endmodule
