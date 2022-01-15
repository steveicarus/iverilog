module top;
  task tsk;
    $display("In task %m()");
  endtask

  function void fnc();
    $display("In function %m()");
  endfunction

  initial begin
    $display("In %m.initial");
    tsk();
    fnc();
    #10;
    $display("Done with simulation at %0d", $time);
  end

  final begin
//    tsk(); // This is from gh442 and is now an error
    fnc();
    $display("In %m.final");
    $display("PASSED");
  end
endmodule
