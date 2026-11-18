// Check a class specialization that is a parameter of a specialization of
// the same class, for a type parameter and for a value parameter.

class C #(type T = int);
  T x;
endclass

class V #(int N = 1);
  logic [N-1:0] v;
endclass

module test;
  localparam int W = 3;
  C#(C#(byte)) c;
  C#(C#(C#(shortint))) d;
  V#(W) a;
  C#(byte) in1;
  C#(C#(shortint)) in2;
  C#(shortint) in3;
  bit failed = 0;

  initial begin
    c = new;
    in1 = new;
    in1.x = 8'h5a;
    c.x = in1;
    d = new;
    in2 = new;
    in3 = new;
    in3.x = 16'h1234;
    in2.x = in3;
    d.x = in2;
    a = new;
    a.v = 3'b101;

    in1 = c.x;
    in2 = d.x;
    in3 = in2.x;
    if (in1.x != 8'h5a || $bits(in1.x) != 8) failed = 1;
    if (in3.x != 16'h1234 || $bits(in3.x) != 16) failed = 1;
    if (a.v != 3'b101) failed = 1;

    if (failed)
      $display("FAILED");
    else
      $display("PASSED");
  end
endmodule
