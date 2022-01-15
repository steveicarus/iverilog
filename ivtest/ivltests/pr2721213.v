module top;
  task foo();
    $display("PASSED");
  endtask

  initial foo;
endmodule
