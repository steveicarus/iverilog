module automatic_static_block_exit;
  integer calls = 0;
  integer completed = 0;

  task run(input bit early_return);
    begin
      automatic integer values[0:1023];
      if (values[0] !== 32'bx) $fatal(1, "context was not reset");
      values[0] = calls;
      calls = calls + 1;
      #1;
      if (early_return) return;
      disable run;
    end
    $fatal(1, "task exit fell through");
  endtask

  task blocked;
    begin
      automatic int value = 7;
      #10;
      $fatal(1, "cancelled task resumed: %0d", value);
    end
  endtask

  initial begin
    repeat (100) begin
      run(1);
      run(0);
    end
    fork
      blocked;
      begin
        #1;
        disable blocked;
      end
    join
    begin
      automatic int value = 9;
      if (value != 9) $fatal(1, "context stack was not restored");
    end
    completed = completed + 1;
  end

  final begin
    if (calls != 200 || completed != 1) $fatal(1, "incorrect task exits");
    $display("PASSED");
  end
endmodule
