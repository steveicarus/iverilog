// Check that an interface port cannot acquire an initializer by continuation.

interface I;
  logic value;
endinterface

module M(I p, q = 1'b0); // Error: interface ports cannot have initializers.
endmodule

module test;

  I i_p();
  I i_q();
  M i_m(i_p, i_q);

endmodule
