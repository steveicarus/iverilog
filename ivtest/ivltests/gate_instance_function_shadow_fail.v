// Check that a later gate instance hides an outer function during call lookup.

module test;
  function integer i_g(input integer value);
    i_g = value;
  endfunction

  if (1) begin : g
    wire out;

    initial $display("%0d", i_g(23)); // Error: i_g is a gate instance, not a function.

    and i_g(out, 1'b1, 1'b1);
  end
endmodule
