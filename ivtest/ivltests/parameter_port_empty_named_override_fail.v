// Check that an empty parameter-port list makes body parameters local.

module M #();
  parameter P = 7;
endmodule

module test;
  M #(.P(9)) i_m(); // Error: A body parameter after #() cannot be overridden.
endmodule
