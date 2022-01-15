`timescale 1ns/100ps

module top;
  reg pass;
  real rarr[0:3];
  realtime del;
  reg signed [1:0] idx;
  integer count;
  event evt;

  initial begin
    pass = 1'b1;

    // Check the initial values.
    if (rarr[1] != 0.0) begin
      $display("FAILED initial value, expected 0.0, got %f", rarr[1]);
      pass = 1'b0;
    end

    // Check a negative index value.
    rarr[3] = 3.0;
    idx = -1;
    rarr[idx] <= 0.0;
    #0.1;
    if (rarr[3] != 3.0) begin
      $display("FAILED negative index 1, expected 3.0, got %f", rarr[3]);
      pass = 1'b0;
    end

    del = 1.0;
    rarr[idx] <= #(del) 0.0;
    #1.1;
    if (rarr[3] != 3.0) begin
      $display("FAILED negative index 2, expected 3.0, got %f", rarr[3]);
      pass = 1'b0;
    end

    count = 0;
    rarr[idx] <= repeat(count) @(evt) 0.0;
    #0.1;
    if (rarr[3] != 3.0) begin
      $display("FAILED negative index 3, expected 3.0, got %f", rarr[3]);
      pass = 1'b0;
    end

    // Check a non-blocking assignment.
    rarr[1] <= 2.0;
    if (rarr[1] != 0.0) begin
      $display("FAILED non-blocking assign 1, expected 0.0, got %f", rarr[1]);
      pass = 1'b0;
    end
    #0.1;
    if (rarr[1] != 2.0) begin
      $display("FAILED non-blocking assign 2, expected 2.0, got %f", rarr[1]);
      pass = 1'b0;
    end

    // Check a delayed non-blocking assignment.
    rarr[1] <= #2 3.0;
    #1.9;
    if (rarr[1] != 2.0) begin
      $display("FAILED delayed NB assign 1, expected 2.0, got %f", rarr[1]);
      pass = 1'b0;
    end
    #0.2;
    if (rarr[1] != 3.0) begin
      $display("FAILED delayed NB assign 2, expected 3.0, got %f", rarr[1]);
      pass = 1'b0;
    end

    // Check a variable delay non-blocking assignment.
    del = 3.0;
    rarr[1] <= #(del) 4.0;
    #2.9;
    if (rarr[1] != 3.0) begin
      $display("FAILED var. delay NB assign 1, expected 3.0, got %f", rarr[1]);
      pass = 1'b0;
    end
    #0.2;
    if (rarr[1] != 4.0) begin
      $display("FAILED var. delay NB assign 2, expected 4.0, got %f", rarr[1]);
      pass = 1'b0;
    end

    // Check a zero count event non-blocking assignment.
    rarr[1] <= repeat(count) @(evt) 5.0;
    #0.1;
    if (rarr[1] != 5.0) begin
      $display("FAILED NB EC count=0, expected 5.0, got %f", rarr[1]);
      pass = 1'b0;
    end

    // Check for an event non-blocking assignment.
    rarr[1] <= @(evt) 6.0;
    fork
      #1 ->evt;
      begin
        #0.9;
        if (rarr[1] != 5.0) begin
          $display("FAILED NB EC initial, expected 5.0, got %f", rarr[1]);
          pass = 1'b0;
        end
        #0.2;
        if (rarr[1] != 6.0) begin
          $display("FAILED NB EC final, expected 6.0, got %f", rarr[1]);
          pass = 1'b0;
        end
      end
    join

    if (pass) $display("PASSED");
 end
endmodule
