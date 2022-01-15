module task_time_arg;
  reg pass;
  integer result;

  task test_it1;
    realtime tmp;
    begin
      tmp = $realtime;
      go_busy(tmp);
    end
  endtask

  task test_it2;
    go_busy($realtime);
  endtask

  task go_busy;
    input delay;
    integer delay;
    result = delay;
  endtask // go_busy

  initial begin
    pass = 1'b1;
    #6
    test_it1;
    if (result !== 6) begin
      $display("Failed: testit1, expected 6, got %d", result);
      pass = 1'b0;
    end

    #1
    test_it2;
    if (result !== 7) begin
      $display("Failed: testit2, expected 7, got %d", result);
      pass = 1'b0;
    end

    if (pass)  $display("PASSED");
  end

endmodule
