typedef logic data_t;

module dut(i, o);

input  data_t i;
output data_t o;

always @* o = i;

endmodule

module test();

data_t i, o;

dut dut(i, o);

reg failed = 0;

initial begin
  i = 1'b0;
  #0 $display(i,,o);
  if (o !== 1'b0) failed = 1;
  i = 1'b1;
  #0 $display(i,,o);
  if (o !== 1'b1) failed = 1;
  i = 1'bx;
  #0 $display(i,,o);
  if (o !== 1'bx) failed = 1;
  i = 1'bz;
  #0 $display(i,,o);
  if (o !== 1'bz) failed = 1;

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
