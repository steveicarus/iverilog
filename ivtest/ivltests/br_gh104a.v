class testclass;
  task start();
    top.dut.signal = 1;
  endtask
endclass

module dut();
  logic signal = 0;

  initial begin
    $display(signal);
    @(signal);
    $display(signal);
    if (signal === 1)
      $display("PASSED");
    else
      $display("FAILED");
    $finish;
  end
endmodule

module top();
  testclass tc;

  initial begin
    #1 tc.start();
  end

  dut dut();
endmodule
