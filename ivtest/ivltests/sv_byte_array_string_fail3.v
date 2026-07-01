// Check that string literals cannot be connected to output byte array ports.

module M (
  output byte out [0:1]
);

  initial begin
    out = "CD";
  end

endmodule

module test;

  M i_m("AB"); // Error: output expression is not assignable

endmodule
