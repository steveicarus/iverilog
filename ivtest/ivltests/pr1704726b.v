module top;
  reg pass = 1'b1;
  parameter parm = 1;

  /***********
   * Check generate tasks.
   ***********/
  // Only one is created.
  generate
    if (parm) begin: name_ti
      task name_task;
        $display("OK in task from scope name_ti");
      endtask
    end else begin: name_ti
      task name_task;
        begin
          $display("FAILED in task from scope name_ti");
          pass = 1'b0;
        end
      endtask
    end
  endgenerate

  // Again only one is created.
  generate
    case (parm)
      1: begin: name_tc
        task name_task;
          $display("OK in task from scope name_tc");
        endtask
      end
      default: begin: name_tc
        task name_task;
          begin
            $display("FAILED in task from scope name_tc");
            pass = 1'b0;
          end
        endtask
      end
    endcase
  endgenerate

  // Two are created, but they are in a different scope.
  genvar lpt;

  generate
    for (lpt = 0; lpt < 2; lpt = lpt + 1) begin: name_tf
      task name_task;
        $display("OK in task from scope name_tf[%0d]", lpt);
      endtask
    end
  endgenerate

  /***********
   * Check functions.
   ***********/
  // Only one is created.
  generate
    if (parm) begin: name_fi
      function name_func;
        input in;
        name_func = ~in;
      endfunction
    end else begin: name_fi
      function name_func;
        input in;
        name_func = in;
      endfunction
    end
  endgenerate

  // Again only one is created.
  generate
    case (parm)
      1: begin: name_fc
        function name_func;
          input in;
          name_func = ~in;
        endfunction
      end
      default: begin: name_fc
        function name_func;
          input in;
          name_func = in;
        endfunction
      end
    endcase
  endgenerate

  // Two are created, but they are in a different scope.
  genvar lpf;

  generate
    for (lpf = 0; lpf < 2; lpf = lpf + 1) begin: name_ff
      function name_func;
        input in;
        name_func = (lpf % 2) ? in : ~in ;
      endfunction
    end
  endgenerate

  initial begin
    name_ti.name_task;
    name_tc.name_task;
    name_tf[0].name_task;
    name_tf[1].name_task;

    if (name_fi.name_func(1'b1) !== 1'b0) begin
      $display("FAILED in function from scope name_fi");
      pass = 1'b0;
    end else $display("OK in function from scope name_fi");

    if (name_fc.name_func(1'b1) !== 1'b0) begin
      $display("FAILED in function from scope name_fc");
      pass = 1'b0;
    end else $display("OK in function from scope name_fc");

    if (name_ff[0].name_func(1'b1) !== 1'b0) begin
      $display("FAILED in function from scope name_ff[0]");
      pass = 1'b0;
    end else $display("OK in function from scope name_ff[0]");

    if (name_ff[1].name_func(1'b1) !== 1'b1) begin
      $display("FAILED in function from scope name_ff[1]");
      pass = 1'b0;
    end else $display("OK in function from scope name_ff[1]");

    if (pass) $display("PASSED");
  end
endmodule
