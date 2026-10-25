// Check that a module instance cannot have two wildcard port connections.

module M(input value);
endmodule

module test;
  wire value = 1'b0;
  M i_m(.*, .*); // Error: Only one wildcard connection is allowed.
endmodule
