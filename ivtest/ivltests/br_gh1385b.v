// Check that enum literals declared inside tasks are available in the task
// scope.

module test;

  reg failed;

  `define check(val, exp) do begin \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end \
  end while (0)

  task check_task_enum;
    typedef enum logic [3:0] {
      A = 4'd1,
      B = 4'd2
    } EN_T;

    static EN_T e = A;

    `check(e, A);

    e = B;
    `check(e, B);
  endtask

  initial begin
    failed = 1'b0;

    #1;
    check_task_enum();

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
