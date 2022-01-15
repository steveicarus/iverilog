primitive latch(q, e, d);

output  q;
input   e;
input   d;

reg     q;

initial q = 1'b0;

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
  r = q;
end

initial begin
  #1;
  // check that the always process executed before the initial process
  if (r === 0)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
