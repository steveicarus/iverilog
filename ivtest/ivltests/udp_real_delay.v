`timescale 1ns/100ps

primitive udp_and(
  output y,
  input  a,
  input  b
);

table
//a b : y
  0 0 : 0 ;
  0 1 : 0 ;
  1 0 : 0 ;
  1 1 : 1 ;
  x 0 : 0 ;
  0 x : 0 ;
endtable

endprimitive

module test();

reg         a;
reg         b;
wire        y;

udp_and #0.5 gate(y, a, b);

reg failed = 0;

initial begin
  $monitor($realtime,,a,,b,,y);
  a = 0; b = 0;
  #0.6 if (y !== 1'b0) failed = 1;
  a = 1; b = 1;
  #0.4 if (y !== 1'b0) failed = 1;
  #0.2 if (y !== 1'b1) failed = 1;
  a = 0; b = 1;
  #0.4 if (y !== 1'b1) failed = 1;
  #0.2 if (y !== 1'b0) failed = 1;
  a = 1; b = 1;
  #0.4 if (y !== 1'b0) failed = 1;
  #0.2 if (y !== 1'b1) failed = 1;
  a = 1; b = 0;
  #0.4 if (y !== 1'b1) failed = 1;
  #0.2 if (y !== 1'b0) failed = 1;

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
