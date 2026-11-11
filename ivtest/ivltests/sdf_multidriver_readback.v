`timescale 1ps/1ps
module driven_cell(input A, E0, E1, output Y);
  bufif1 first(Y, A, E0);
  bufif1 second(Y, A, E1);
  specify
    (A => Y) = (2, 3);
  endspecify
endmodule

module top;
  reg A = 0, E0 = 0, E1 = 0;
  wire left_y, right_y;
  integer left_events = 0, right_events = 0;
  driven_cell left(A, E0, E1, left_y);
  driven_cell right(A, E0, E1, right_y);

  initial begin
    #1;
    $sdf_annotate("ivltests/sdf_multidriver_readback.sdf", top);
    if (!$check_multidriver_paths("top.left", 7, 11) ||
        !$check_multidriver_paths("top.right", 17, 19))
      $fatal(1, "FAIL module-path readback");
    #19 E0 = 1;
    #80 A = 1;
    #100 A = 0;
    #100 E0 = 0;
    #100 E1 = 1;
    #100 A = 1;
    #100 A = 0;
    #50;
    if (left_events != 4 || right_events != 4)
      $fatal(1, "FAIL event counts: %0d %0d", left_events, right_events);
    if (!$check_multidriver_paths("top.left", 7, 11) ||
        !$check_multidriver_paths("top.right", 17, 19))
      $fatal(1, "FAIL final module-path readback");
    $display("PASSED");
    $finish(0);
  end

  // Only one driver is enabled in each measured data interval. These checks
  // observe the public output; they are not per-driver callback assertions.
  always @(left_y) if (($time > 90 && $time < 300) || $time > 490) begin
    left_events = left_events + 1;
    if (!((left_events == 1 && left_y === 1'b1 && $time == 107) ||
          (left_events == 2 && left_y === 1'b0 && $time == 211) ||
          (left_events == 3 && left_y === 1'b1 && $time == 507) ||
          (left_events == 4 && left_y === 1'b0 && $time == 611)))
      $fatal(1, "FAIL left event %0d at %0t = %b", left_events, $time, left_y);
  end
  always @(right_y) if (($time > 90 && $time < 300) || $time > 490) begin
    right_events = right_events + 1;
    if (!((right_events == 1 && right_y === 1'b1 && $time == 117) ||
          (right_events == 2 && right_y === 1'b0 && $time == 219) ||
          (right_events == 3 && right_y === 1'b1 && $time == 517) ||
          (right_events == 4 && right_y === 1'b0 && $time == 619)))
      $fatal(1, "FAIL right event %0d at %0t = %b", right_events, $time, right_y);
  end
endmodule
