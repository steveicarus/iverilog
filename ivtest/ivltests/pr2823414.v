module top;

  initial begin
`ifdef CAUSES_PROBLEM
    /*
     * C-Style comment in a skipped `ifdef is loosing the '\n'.
     */
`endif
  // This should report an error at line 10.
  fail_at_line_10();
  end

endmodule
