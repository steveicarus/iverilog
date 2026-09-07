// Check that a switch instance hides an outer variable.

module test;
  integer i_g = 23;

  if (1) begin : g
    wire out;
    wire in = 1'b1;
    tran i_g(out, in);

    initial $display("%0d", i_g); // Error: i_g is a switch instance, not a value.
  end
endmodule
