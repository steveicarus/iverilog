module top;
  initial fork : named_fork
    $display("PASSED");
  join : named_fork
endmodule
