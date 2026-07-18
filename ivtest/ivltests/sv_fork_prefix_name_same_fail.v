// Check that matching prefix and post-fork labels are rejected.

module test;
  initial LABEL: fork : LABEL
  join
endmodule
