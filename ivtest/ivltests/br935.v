module top;
  reg test;
  event ev;
  reg val;

  initial begin
    #10;
    test = 1'b1;
    #20;
    $display("FAILED");
    // I would expect this watchdog to require a $finish() since the other
    // initial did not finish, but it's not needed so that implies the
    // other initial is getting disabled not just the named begin.
  end

  initial begin
    test = 1'b0;
    val = 1'b0;
    fork
      // With the @ line here development fails. If it is after the named
      // begin it passes.
      @(test) disable nb;
      begin : nb
        // Any blocking item here causes the problem.
//        wait(val);
//        @(ev);
        #20;
      end
    join
    // This is never executed even though it should run at time 10.
    $display("PASSED");
    // The finish is required to prevent the watchdog from running.
    $finish;
  end
endmodule
