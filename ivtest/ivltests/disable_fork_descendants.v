// Check that disable fork also terminates the detached children of the
// calling process's children (IEEE 1800-2017 9.6.3: "terminates all
// active descendants").

`timescale 1ns/1ns

module test;

  reg failed = 0;
  reg grandchild = 0;
  reg great_grandchild = 0;
  reg joined_grandchild = 0;
  reg other_process = 0;

  // The child is still running when its parent executes disable fork.
  initial begin
    fork
      begin
        fork #3 grandchild = 1; join_none
        #5;
      end
    join_none
    #1 disable fork;
  end

  // A detached child that is below a joined child.
  initial begin
    fork
      begin
        fork
          begin
            fork #3 great_grandchild = 1; join_none
            #5;
          end
        join
      end
    join_none
    #1 disable fork;
  end

  // A joined grandchild was already terminated before this fix.
  initial begin
    fork
      begin
        fork #3 joined_grandchild = 1; join
      end
    join_none
    #1 disable fork;
  end

  // Children of another process must not be terminated.
  initial begin
    fork #3 other_process = 1; join_none
  end

  initial begin
    #4;
    if (grandchild !== 0) begin
      $display("FAILED: detached grandchild was not disabled");
      failed = 1;
    end
    if (great_grandchild !== 0) begin
      $display("FAILED: detached child below a joined child was not disabled");
      failed = 1;
    end
    if (joined_grandchild !== 0) begin
      $display("FAILED: joined grandchild was not disabled");
      failed = 1;
    end
    if (other_process !== 1) begin
      $display("FAILED: child of another process was disabled");
      failed = 1;
    end
    if (!failed)
      $display("PASSED");
  end

endmodule
