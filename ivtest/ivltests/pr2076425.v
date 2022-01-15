module top;

  task delay;
    z.delay;
  endtask

  always begin
    delay;
  end

  initial begin
    #10 $display("PASSED");
    $finish;
  end
endmodule

module z;
  task delay;
    #1;
  endtask
endmodule
