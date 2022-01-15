`define display_passed \
  initial begin // comment \
    $display("PASSED"); \
  end

module test();

`display_passed

endmodule
