// Supply a second compilation unit with a different value of VALUE.

parameter integer VALUE = 23;

module M(output wire [31:0] value, unit_value);
  assign value = VALUE;
  assign unit_value = $unit::VALUE;
endmodule
