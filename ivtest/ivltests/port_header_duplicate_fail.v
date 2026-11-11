// Check that explicit port names cannot be repeated in a module header.

module M(.p(a), .p(b)); // Error: The port name p is already defined.
  input a, b;
endmodule

module test;
  M i_m(1'b0, 1'b1);
  initial $display("FAILED");
endmodule
