// Icarus 0.6, snapshot 20020907
// ==================================================
// -- confused by disables from within a fork -- vvp fails
//
// -- to run, incant
//                  iverilog tt.v
//                  vvp a.out

module top;

  integer simple_fail, loop_fail, fork_fail, tlp_fail, tfk_fail;
  integer loop_cntr, tlp_cntr;
  reg fred, abort;

  initial begin
    #1;
    simple_fail = 0;
    loop_fail = 0;
    fork_fail = 0;
    tlp_fail = 0;
    tfk_fail = 0;
    fred = 0;
    abort = 1;
    #4;
    fred = 1;
    #4
    if(simple_fail) $display("\n***** simple block disable FAILED *****");
    else            $display("\n***** simple block disable PASSED *****");
    if(loop_fail) $display("***** block with loop disable FAILED *****");
    else          $display("***** block with loop disable PASSED *****");
    if(fork_fail) $display("***** forked block disable FAILED *****");
    else          $display("***** forked block disable PASSED *****");
    if(tlp_fail) $display("***** task with loop disable FAILED *****");
    else         $display("***** task with loop disable PASSED *****");
    if(tfk_fail) $display("***** task with forked block disable FAILED ****\n");
    else         $display("***** task with forked block disable PASSED ****\n");
    $finish(0);
  end

  // simple block disable
  initial begin: block_name
    #2;
    disable block_name;
    simple_fail = 1;
  end

  // more complex: block disable inside for-loop
  initial begin
    #2;
    begin: configloop
      for (loop_cntr = 0; loop_cntr < 3; loop_cntr=loop_cntr+1) begin
	wait (fred);
	if (abort) begin
	  disable configloop;
	end
	loop_fail = 1;
      end
    end // configloop block
    if (loop_fail) $display("\n\ttime: %0t, loop_cntr: %0d",$time,loop_cntr);
  end

  // still more complex: disable from within a forked block
  initial begin
    #2;
    begin: forked_tasks
      fork
	begin
	  #5;
	  fork_fail = 1;
	end
	begin
	  @(fred);
	  disable forked_tasks;
	  fork_fail = 1;
	end
      join
      fork_fail = 1;
    end //forked_tasks
  end

  // disables inside tasks
  initial begin
    task_with_loop;
  end
  initial begin
    task_with_fork;
  end

task task_with_loop;
  begin
    #2;
    begin: configtlp
      for (tlp_cntr = 0; tlp_cntr < 3; tlp_cntr=tlp_cntr+1) begin
	wait (fred);
	if (abort) begin
	  disable configtlp;
	end
	tlp_fail = 1;
      end
    end // configloop block
  end
endtask // task_with_loop

task task_with_fork;
  begin
    #2;
    begin: forked_tasks_in_task
      fork
	begin
	  #5;
	  tfk_fail = 1;
	end
	begin
	  @(fred);
	  disable forked_tasks_in_task;
	  tfk_fail = 1;
	end
      join
      tfk_fail = 1;
    end //forked_tasks_in_task
  end
endtask // task_with_fork

endmodule
