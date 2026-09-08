// Check that $unit is resolved in the compilation unit of each module.

parameter integer VALUE = 42;

module test;
  wire [31:0] value, unit_value;
  M i_m(value, unit_value);
  initial begin
    #1;
    if ($unit::VALUE == 42 && value == 23 && unit_value == 23) $display("PASSED");
    else $display("FAILED: unit=%0d module=%0d/%0d",
                  $unit::VALUE, value, unit_value);
  end
endmodule
