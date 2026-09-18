// Check that a root module name cannot have an instance select.

module M;
  reg value;
endmodule

module test;
  initial $display("%b", M[1].value); // Error: Root module M is not an instance array.
endmodule
