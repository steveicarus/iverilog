module test();

task t(input integer a, integer b);
  $display(a,,b);
endtask

initial t(0, 1);

endmodule
