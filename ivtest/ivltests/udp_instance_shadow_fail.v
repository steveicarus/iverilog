// Check that a UDP instance hides an outer variable.

primitive P(out, in);
  output out;
  input in;

  table
    0 : 0;
    1 : 1;
  endtable
endprimitive

module test;
  integer i_p = 23;

  if (1) begin : g
    wire out;
    P i_p(out, 1'b1);

    initial $display("%0d", i_p); // Error: i_p is a UDP instance, not a value.
  end
endmodule
