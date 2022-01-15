module test();

task t(a, b);
  $display(a,,b);
endtask

initial t(0, 1);

endmodule
