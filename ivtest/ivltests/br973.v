// Regression test for bug #973

module test();

typedef enum bit   { A0, A1 } A;
typedef enum logic { B0, B1 } B;
typedef enum reg   { C0, C1 } C;

A enum1;
B enum2;
C enum3;

initial begin
  if ($bits(enum1) == 1 && $bits(enum2) == 1 && $bits(enum3) == 1)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
