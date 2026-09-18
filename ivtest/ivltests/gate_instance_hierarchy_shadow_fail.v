// Check that a gate instance hides an outer module instance in a dotted name.

module M;
  parameter value = 23;
endmodule

module test;
  M i_g();

  if (1) begin : g
    wire out;
    and i_g(out, 1'b1, 1'b1);

    initial $display("%0d", i_g.value); // Error: i_g is a gate instance, not a scope.
  end
endmodule
