// Check that a UDP instance hides an outer typedef used as a cast target.

typedef logic [7:0] i_p;

primitive P(out, in);
  output out;
  input in;
  table
    0 : 0;
    1 : 1;
  endtable
endprimitive

module test;
  if (1) begin : g
    wire out;

    P i_p(out, 1'b1); // Instantiates primitive and shadows the outer typedef.
    wire [7:0] value = i_p'(8'h2a); // Error: i_p names the UDP instance, not a type.
  end
endmodule
