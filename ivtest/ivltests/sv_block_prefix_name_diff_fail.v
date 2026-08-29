// Check that different prefix and post-begin labels are rejected.

module test;
  initial LABEL_A: begin : LABEL_B
  end
endmodule
