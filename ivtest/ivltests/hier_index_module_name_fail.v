// Check that a module definition name cannot have an instance select.

module M;
  reg value;

  initial $display("%b", M[1].value); // Error: M is not an instance array.
endmodule

module test;
  M m();
endmodule
