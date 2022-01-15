module test();

real val;

always @(val) begin
  $display("val = %f", val);
end

task automatic test_task(input integer delay, input real val1, input real val2);

real val;

fork
  begin
    @(val) $display("val%0d = %f", delay, val);
    @(val) $display("val%0d = %f", delay, val);
  end
  begin
    #delay;
    #2 val = val1;
    #2 val = val1;
    #2 val = val2;
  end
join

endtask

initial begin
  #1 val = 1.0;
  #1 val = 1.0;
  #1 val = 2.0;
  fork
    test_task(1, 1.1, 2.1);
    test_task(2, 1.2, 2.2);
  join
  fork
    test_task(1, 2.1, 3.1);
    test_task(2, 2.2, 3.2);
  join
  #1 $finish(0);
end

endmodule
