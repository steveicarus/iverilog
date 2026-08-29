// Check that assertion labels can shadow type identifiers after declarations.

typedef reg [7:0] CHECK;

module test;

  task check_task;
    CHECK value;
    CHECK: assert (1);
  endtask

  initial begin
    begin
      CHECK value;
      CHECK: assert (1);
    end

    check_task;
    $display("PASSED");
  end

endmodule
