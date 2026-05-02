// Check that an output array port expression must be assignable.

module M (
  output int out [0:1]
);

  initial begin
    out = '{3, 4};
  end

endmodule

module test;

  M i_m('{1, 2}); // Error: output expression is not assignable

endmodule
