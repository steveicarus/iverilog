package pkg1;

typedef enum logic [1:0] {
  R0     = 2'b00,
  R1     = 2'b01,
  R2     = 2'b10,
  R3     = 2'b11
} reg_t;

endpackage

module dut(input pkg1::reg_t r1, output pkg1::reg_t r3);

import pkg1::*;

reg_t r2;

always_comb
  r2 = r1;

always_comb
  r3 = r2;

endmodule


module test();

import pkg1::*;

reg_t v1;
reg_t v2;

dut dut(v1, v2);

reg failed = 0;

initial begin
  v1 = R0;
  #1 $display("%h %h", v1, v2);
  if (v2 !== R0) failed = 1;
  v1 = R1;
  #1 $display("%h %h", v1, v2);
  if (v2 !== R1) failed = 1;
  v1 = R2;
  #1 $display("%h %h", v1, v2);
  if (v2 !== R2) failed = 1;
  v1 = R3;
  #1 $display("%h %h", v1, v2);
  if (v2 !== R3) failed = 1;

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
