// Check that a packed union rejects duplicate names in one declaration.

module test;
  union packed {
    reg value, value; // Error: The union already has a member named value.
  } data;

  initial $display("FAILED");
endmodule
