// Check that an implicit port name cannot conflict with an explicit port.

module M(.a(b), a); // Error: The port name a is already defined.
  input a, b;
endmodule

module test;
  M i_m(1'b0, 1'b1);
  initial $display("FAILED");
endmodule
