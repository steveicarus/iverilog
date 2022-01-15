module top;

  /***********
   * Check parameters.
   ***********/
  // Check parameter/parameter name issues.
  parameter name_pp = 1;
  parameter name_pp = 0;

  parameter name_pl = 1;
  localparam name_pl = 0;

  localparam name_lp = 0;
  parameter name_lp = 1;

  localparam name_ll = 1;
  localparam name_ll = 0;

  /***********
   * Check genvars.
   ***********/
  // Check genvar/genvar name issues.
  genvar name_vv;
  genvar name_vv;

  /***********
   * Check tasks.
   ***********/
  // Check task/task name issues.
  task name_tt;
    $display("FAILED in task name_tt(a)");
  endtask
  task name_tt;
    $display("FAILED in task name_tt(b)");
  endtask

  // Check that task/task checks work in a generate block.
  generate
    begin: task_blk
      task name_tt;
        $display("FAILED in task name_tt(a)");
      endtask
      task name_tt;
        $display("FAILED in task name_tt(b)");
      endtask
    end
  endgenerate

  /***********
   * Check functions.
   ***********/
  // Check function/function name issues.
  function name_ff;
    input in;
    name_ff = in;
  endfunction
  function name_ff;
    input in;
    name_ff = 2*in;
  endfunction

  // Check that function/function checks work in a generate block.
  generate
    begin: task_blk
      function name_ff;
        input in;
        name_ff = in;
      endfunction
      function name_ff;
        input in;
        name_ff = 2*in;
      endfunction
    end
  endgenerate

  /***********
   * Check named events
   ***********/
  // Check named event/named event name issues.
  event name_ee;
  event name_ee;

  initial name_tt;

  specify
    specparam name_ss = 1;
    specparam name_ss = 0;
  endspecify
endmodule
