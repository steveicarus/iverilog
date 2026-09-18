// Check that a type parameter prevents a wildcard port connection from
// binding an outer variable.

module M(input [31:0] value);
endmodule

reg [31:0] value;

module test;

  parameter type value = logic;

  M i_m(.*);

endmodule
