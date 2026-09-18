// Check that a UDP instance hides an outer parameter during enum elaboration.

parameter i_p = 8;

primitive P(out, in);
  output out;
  input in;

  table
    0 : 0;
    1 : 1;
  endtable
endprimitive

module test;
  wire out;
  P i_p(out, 1'b1);

  // Enum ranges are resolved during scope elaboration, before signals.
  typedef enum logic [i_p-1:0] { A = 0 } T; // Error: i_p is a UDP instance, not a constant.
endmodule
