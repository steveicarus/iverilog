// Ensure the compiler doesn't perform some invalid optimisations.

module test();

reg [3:0] unknown;
reg [3:0] result;

reg failed;

initial begin
  failed = 0;
  unknown = 4'bx101;
  result = unknown + 0;
  $display("%b", result);
  if (result !== 4'bxxxx) failed = 1;
  result = (unknown >> 1) + 0;
  $display("%b", result);
  if (result !== 4'bxxxx) failed = 1;
  result = unknown - 0;
  $display("%b", result);
  if (result !== 4'bxxxx) failed = 1;
  result = unknown * 0;
  $display("%b", result);
  if (result !== 4'bxxxx) failed = 1;

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
