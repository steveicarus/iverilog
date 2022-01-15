// Icarus 0.6, snapshot 20020907
// ==================================================
// -- confused by disables from within a fork -- vvp fails
//
// -- to run, incant
//                  iverilog tt.v
//                  vvp a.out
//
// Veriwell
// ========
// -- OK
//
module top;

  integer simple_fail, loop_fail, fork_fail, tlp_fail;
  integer tfk_fail, tfk2_fail, tfk3_fail;
  integer tfk2pos, tfk2nega, tfk2negb;
  integer tfk3pos, tfk3nega, tfk3negb;
  integer loop_cntr, tlp_cntr;
  reg fred, abort;

  initial begin
    #1;
    simple_fail = 0;
    loop_fail = 0;
    fork_fail = 0;
    tlp_fail = 0;
    tfk_fail = 0;
    tfk2_fail = 0;
    tfk2pos = 0;
    tfk2nega = 1;
    tfk2negb = 1;
    tfk3pos = 0;
    tfk3nega = 1;
    tfk3negb = 1;
    fred = 0;
    abort = 1;
    #4;
    fred = 1;
    #4
    $display("Check disable:");
    if(simple_fail) $display("***** simple block           FAILED *****");
    else            $display("***** simple block           PASSED *****");
    if(loop_fail)   $display("***** block with loop        FAILED *****");
    else            $display("***** block with loop        PASSED *****");
    if(fork_fail)   $display("***** forked block           FAILED *****");
    else            $display("***** forked block           PASSED *****");
    if(tlp_fail)    $display("***** task with loop         FAILED *****");
    else            $display("***** task with loop         PASSED *****");
    if(tfk_fail)    $display("***** task with forked block FAILED *****");
    else            $display("***** task with forked block PASSED *****");
    if(tfk2_fail)   $display("***** one forked block       FAILED *****");
    else            $display("***** one forked block       PASSED *****");
    if(tfk3_fail)   $display("***** the other forked block FAILED *****");
    else            $display("***** the other forked block PASSED *****");
    $display("");
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
  initial begin
    task_with_fork2;
    if(tfk2pos || tfk2nega || tfk2negb) tfk2_fail = 1;
  end
  initial begin
    task_with_fork3;
    if(tfk3pos || tfk3nega || tfk3negb) tfk3_fail = 1;
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

task task_with_fork;  // disable block whick calls fork
  begin
    #2;
    begin: forked_tasks_in_task
      fork
	begin: alf
	  #5;
	  tfk_fail = 1;
	end
	begin: bet
	  @(fred);
	  disable forked_tasks_in_task;
	  tfk_fail = 1;
	end
      join
      tfk_fail = 1;
    end //forked_tasks_in_task
  end
endtask // task_with_fork

task task_with_fork2; // disable *one* of the forked blocks
  begin
    #2;
    begin: forked_tasks_in_task2
      fork
	begin: gam
	  #5;
	  tfk2pos = 1;
	end
	begin: delt
	  @(fred);
	  disable gam;
	  tfk2nega = 0;
	end
      join
      tfk2negb = 0;
    end //forked_tasks_in_task
  end
endtask // task_with_fork

task task_with_fork3; // disable *one* of the forked blocks
  begin
    #2;
    begin: forked_tasks_in_task3
      fork
	begin: eps
	  #5;
	  tfk3nega = 0;
	end
	begin: zet
	  @(fred);
	  disable zet;
	  tfk3pos = 1;
	end
      join
      tfk3negb = 0;
    end //forked_tasks_in_task
  end
endtask // task_with_fork

endmodule
