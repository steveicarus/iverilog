// Check that assertion labels can use the same names as outer typedefs.

typedef reg [7:0] CHECK;

module test;

  task check_task;
    CHECK: assert (1);
  endtask

  initial begin
    begin
      CHECK: assert (1);
    end

    check_task;
    $display("PASSED");
  end

endmodule
