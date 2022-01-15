module dut;

integer id;

endmodule

module test;

dut inst[4];

integer i;

reg failed = 0;

initial begin
  inst[0].id = 0;
  inst[1].id = 1;
  inst[2].id = 2;
  inst[3].id = 3;

  if (inst[0].id !== 0) failed = 1;
  if (inst[1].id !== 1) failed = 1;
  if (inst[2].id !== 2) failed = 1;
  if (inst[3].id !== 3) failed = 1;

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
