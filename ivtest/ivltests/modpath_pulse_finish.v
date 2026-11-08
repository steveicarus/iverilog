// Terminate with both canceled and surviving module-path callbacks pending.
// This checks normal finish only; memory lifetime is reviewed/tested separately.
`timescale 1ps/1ps

module finish_path(input i, output o);
  assign o = i;
  specify
    (i => o) = (100, 0);
  endspecify
endmodule

module test;
  reg i;
  wire o;
  integer changes = 0;
  finish_path dut(i, o);
  always @(o) begin
    if ($time != 0 || o !== 1'b0)
      $fatal(1, "unexpected output before finish");
    changes = changes + 1;
  end
  initial begin
    i = 0;
    #10 i = 1;
    #10 i = 0;
    #10 i = 1;
    #10;
    if (changes != 1 || o !== 1'b0) $fatal(1, "initial output missing");
    $display("PASSED");
    $finish(0);
  end
endmodule
