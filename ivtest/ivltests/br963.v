// Regression test for br962 - based on test case provided in bug report
module qtest;
  parameter width = 32;
  parameter depth = 32;

  reg [width-1:0] values[$];
  reg [$clog2(depth)+width-1:0] sum1;
  reg [$clog2(depth)+width-1:0] sum2;

  task new_sample;
    input [width-1:0] data;
    int i;
    begin
      reg [width-1:0] popped;
      if (values.size >= depth)
	begin : foo
	  popped = values.pop_back();
	  sum1 = sum1 - popped;
	end
      sum1 = sum1 + data;
      values.push_front(data);
      sum2 = 0;
      for (i = 0; i < values.size; i++) begin
        sum2 = sum2 + values[i];
      end
      $display("sum1 = %d  sum2 = %d", sum1, sum2);
      if (sum1 !== sum2) begin
        $display("FAILED");
        $finish;
      end
    end
  endtask

  initial begin
    sum1 = 0;
    repeat (2*depth) new_sample({$random});
    $display("PASSED");
  end

endmodule
