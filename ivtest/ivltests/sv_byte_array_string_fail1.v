// Check that string literals cannot be assigned to unpacked arrays whose
// element type is 4-state.

module test;

  logic [7:0] value [0:3] = "AB"; // Error: target element type is 4-state

endmodule
