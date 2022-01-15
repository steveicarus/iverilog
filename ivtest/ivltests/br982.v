module example;
  task simple_task;
    input in;
    output out;
    begin
      out = in;
    end
  endtask

  reg x = 0;
  initial begin
    simple_task(x);
    $finish;
  end
endmodule
