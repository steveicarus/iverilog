// Check that a packed struct rejects duplicate member names.

module test;
  struct packed {
    reg value;
    reg value; // Error: The struct already has a member named value.
  } data;

  initial $display("FAILED");
endmodule
