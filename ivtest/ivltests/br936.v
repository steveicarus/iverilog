module top;
  task automatic worker(int delay);
    #delay $display("worker finished at %0d.", delay);
  endtask

  initial begin
    fork
      worker(10);
      worker(5);
    join_any
    $display("fork has joined (any)");
    if ($time != 5) begin
       $display("FAILED -- time=%0t", $time);
       $finish;
    end
    $display("PASSED");
  end
endmodule
