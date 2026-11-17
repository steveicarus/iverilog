// Check that the type of a typed pattern is a type name or an integer atom
// type, not a vector with a range or a real type.

module test;

  logic [7:0] v;

  initial begin
    v = bit [7:0]'{4'h1, 4'h2};
  end

endmodule
