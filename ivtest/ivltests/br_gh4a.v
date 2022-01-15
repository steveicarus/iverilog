module test;

  initial fork
    reg a;
    a = 1'b0;
    $display("PASSED");
  join

endmodule
