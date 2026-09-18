// Check that a gate instance array hides an outer variable.

module test;
  integer i_g = 23;

  if (1) begin : g
    wire [1:0] out;
    and i_g[1:0](out, 2'b11, 2'b11);

    initial $display("%0d", i_g); // Error: i_g is a gate instance array, not a value.
  end
endmodule
