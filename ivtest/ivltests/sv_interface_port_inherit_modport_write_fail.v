// Check that an inherited modport still prevents writes to its input members.

interface I;
  logic value;
  modport mp(input value);
endinterface

module M(I.mp p, q);

  assign q.value = 1'b1; // Error: value is an input in the inherited modport.

endmodule

module test;

  I i_p();
  I i_q();
  M i_m(i_p, i_q);

endmodule
