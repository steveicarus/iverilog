`timescale 1ps / 1ps

module test();
  reg[31:0] arr[1:0];

  initial begin
    arr[0] = 'd1;
    arr[1] = 'd5;
    if (arr[0] + 1 + 1 > 4) $display("FAILED"); else $display("PASSED");
  end

endmodule
