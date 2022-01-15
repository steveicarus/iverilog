primitive latch(q, e, d);

output  q;
input   e;
input   d;

reg     q;

table
// e  d | q | q+ |
   1  1 : ? : 1 ;
   1  0 : ? : 0 ;
   0  ? : ? : - ;
endtable

endprimitive

module test();

wire    q;
reg     e;
reg     d;
reg     r;

latch latch(q, e, d);

always @(q) begin
  r = 1;
end

initial begin
  #1;
  $display("%b %b", q, r);
  // the 'x' should propagate to q before the start of simulation
  if (r === 1'bx && q === 1'bx)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
