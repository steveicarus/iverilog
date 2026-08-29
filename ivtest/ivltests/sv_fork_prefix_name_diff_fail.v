// Check that different prefix and post-fork labels are rejected.

module test;
  initial LABEL_A: fork : LABEL_B
  join
endmodule
