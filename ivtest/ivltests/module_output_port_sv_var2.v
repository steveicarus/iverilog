// Check that non-ANSI output ports that have a SystemVerilog data type are
// elaborated as variables and be assigned a value.

typedef struct packed { int x; } T1;
typedef enum { A } T2;
typedef T1 [1:0] T3;

module test1;
output reg a;
output reg [1:0] b;
output integer c;
output time d;
output bit e;
output logic f;
output shortint g;
output int h;
output longint i;
output real r;
output T1 x;
output T2 y;
output T3 z;

initial begin
  a = '0;
  b = '0;
  c = '0;
  d = '0;
  e = '0;
  f = '0;
  g = '0;
  h = '0;
  r = 0.0;
  x = '0;
  y = A;
  z = '0;
  $display("PASSED");
end

endmodule
