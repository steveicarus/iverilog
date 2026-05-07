// Check that string literals cannot be assigned to unpacked arrays whose
// element type is wider than 8 bits.

module test;

  bit [8:0] value [0:3] = "AB"; // Error: target element type is too wide

endmodule
