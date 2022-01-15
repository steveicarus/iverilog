package MyPackage;

typedef enum logic [1:0] {
  A = 2'b00,
  B = 2'b01,
  C = 2'b10
} MyEnum;

endpackage

module test();

import MyPackage::*;

localparam MyB = B;

localparam C = 4;

initial begin
  $display("B = %0d", MyB);
  $display("C = %0d", C);
  if (MyB === 1 && C === 4)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
