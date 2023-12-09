/*
 * From IEEE 1800-2012
 *
 * 5.8 Time literals
 *
 * Time is written in integer or fixed-point format, followed without a space by a
 * time unit ( fs ps ns us ms s ).
 * For example:
 *   2.1ns
 *   40ps
 * The time literal is interpreted as a realtime value scaled to the current time
 * unit and rounded to the current time precision.
 */

module same;
  timeunit 1ps;
  timeprecision 1ps;

  logic passed;

  function logic check_time;
    realtime result;

    check_time = 1'b1;

    result = 1ns;
    if (result != 1000.0) begin
      $display("Failed-same: Expected 1ns to be rounded to 1000.0, got %f", result);
      check_time = 1'b0;
    end

    result = 1ps;
    if (result != 1.0) begin
      $display("Failed-same: Expected 1ps to be rounded to 1.0, got %f", result);
      check_time = 1'b0;
    end

    result = 0.5ps;
    if (result != 1.0) begin
      $display("Failed-same: Expected 0.5ps to be rounded to 1.0, got %f", result);
      check_time = 1'b0;
    end

    result = 0.499ps;
    if (result != 0.0) begin
      $display("Failed-same: Expected 0.49ps to be rounded to 0.0, got %f", result);
      check_time = 1'b0;
    end
  endfunction

  initial begin
    passed = 1'b1;
    #0.499ps;
    if ($realtime() != 0.0) begin
      $display("Failed: Expected a 0.499ps delay to scale to 0ps of delay, got %f", $realtime());
      passed = 1'b0;
    end

    #0.5ps;
    if ($realtime() != 1.0) begin
      $display("Failed: Expected a 0.5ps delay to scale to 1ps of delay, got %f", $realtime());
      passed = 1'b0;
    end
  end
endmodule

module max;
  timeunit 100s;
  timeprecision 1fs;

  bit passed;

  function logic check_time;
    realtime result;

    check_time = 1'b1;

    result = 1s;
    if (result != 1e-2) begin
      $display("Failed-max: Expected 1s to be rounded to 1.0e-2, got %f", result);
      check_time = 1'b0;
    end

    result = 0.5fs;
    if (result != 1e-17) begin
      $display("Failed-max: Expected 0.5fs to be rounded to 1.0e-17, got %f", result);
      check_time = 1'b0;
    end

    result = 0.499fs;
    if (result != 0.0) begin
      $display("Failed-max: Expected 0.49fs to be rounded to 0.0, got %f", result);
      check_time = 1'b0;
    end
  endfunction

  initial begin
    passed = 1'b1;
    #0.499fs;
    if ($realtime() != 0.0) begin
      $display("Failed: Expected a 0.499fs delay to scale to 0s of delay, got %f", $realtime());
      passed = 1'b0;
    end

    #0.5fs;
    if ($realtime() != 1e-17) begin
      $display("Failed: Expected a 0.5fs delay to scale to 1e-17s of delay, got %f", $realtime());
      passed = 1'b0;
    end
  end
endmodule

module top;
  timeunit 1ns;
  timeprecision 1ps;

  realtime result;
  logic passed;

  initial begin
    passed = 1'b1;

    result = 1ns;
    if (result != 1.0) begin
      $display("Failed: Expected 1ns to be rounded to 1.0, got %f", result);
      passed = 1'b0;
    end

    result = 1ps;
    if (result != 0.001) begin
      $display("Failed: Expected 1ps to be rounded to 0.001, got %f", result);
      passed = 1'b0;
    end

    result = 1.23456789ps;
    if (result != 0.001) begin
      $display("Failed: Expected 1.23456789ps to be rounded to 0.001, got %f", result);
      passed = 1'b0;
    end

    result = 0.5ps;
    if (result != 0.001) begin
      $display("Failed: Expected 0.5ps to be rounded to 0.001, got %f", result);
      passed = 1'b0;
    end

    result = 0.499ps;
    if (result != 0.0) begin
      $display("Failed: Expected 0.49ps to be rounded to 0.0, got %f", result);
      passed = 1'b0;
    end

    passed &= same.check_time();
    passed &= max.check_time();

    #0.499ps;
    if ($realtime() != 0.0) begin
      $display("Failed: Expected a 0.499fs delay to scale to 0ns of delay, got %f", $realtime());
      passed = 1'b0;
    end

    #0.5ps;
    if ($realtime() != 0.001) begin
      $display("Failed: Expected a 0.5fs delay to scale to 0.001ns of delay, got %f", $realtime());
      passed = 1'b0;
    end

    #1; // Make sure all the delay tests have completed

    if (passed && max.passed && same.passed) $display("PASSED");
  end
endmodule
