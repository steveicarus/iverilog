// Check that a parameter cannot be assigned twice by name.

module M #(parameter VALUE = 1);
endmodule

module test;

  M #(.VALUE(2), .VALUE(3)) i_m(); // Error: VALUE is assigned more than once.

endmodule
