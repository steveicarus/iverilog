/*
 * For a non-blocking delay the last NB assign in the same thread
 * at the same time must set the final result, but you are allowed
 * to have multiple assignments in the queue.
 */
module top;
  reg passed = 1'b1;
  reg out;
  real rout;
  integer delay;

  initial begin
    out <= 1'b1;
    out <= 1'b0;
    rout <= 0.0;
    rout <= 1.0;
    #1;
    if (out !== 1'b0) begin
      $display("FAILED: zero delay, expected 1'b0, got %b", out);
      passed = 1'b0;
    end
    if (rout != 1.0) begin
      $display("FAILED: zero delay (real), expected 1.0, got %f", rout);
      passed = 1'b0;
    end

    out <= #1 1'b0;
    out <= #1 1'b1;
    rout <= #1 0.0;
    rout <= #1 2.0;
    #2;
    if (out !== 1'b1) begin
      $display("FAILED: constant delay, expected 1'b1, got %b", out);
      passed = 1'b0;
    end
    if (rout != 2.0) begin
      $display("FAILED: constant delay (real), expected 2.0, got %f", rout);
      passed = 1'b0;
    end

    delay = 2;
    out <= #(delay) 1'b1;
    out <= #(delay) 1'b0;
    rout <= #(delay) 0.0;
    rout <= #(delay) 3.0;
    #(delay+1);
    if (out !== 1'b0) begin
      $display("FAILED: calculated delay, expected 1'b0, got %b", out);
      passed = 1'b0;
    end
    if (rout != 3.0) begin
      $display("FAILED: calculated delay (real), expected 3.0, got %f", rout);
      passed = 1'b0;
    end

    out <= #1 1'b1;
    out <= #3 1'b0;
    out <= #5 1'b1;
    rout <= #1 1.0;
    rout <= #3 3.0;
    rout <= #5 5.0;
    #2;
    if (out !== 1'b1) begin
      $display("FAILED: first delay, expected 1'b1, got %b", out);
      passed = 1'b0;
    end
    if (rout != 1.0) begin
      $display("FAILED: first delay (real), expected 1.0, got %f", rout);
      passed = 1'b0;
    end
    #2;
    if (out !== 1'b0) begin
      $display("FAILED: second delay, expected 1'b0, got %b", out);
      passed = 1'b0;
    end
    if (rout != 3.0) begin
      $display("FAILED: second delay (real), expected 3.0, got %f", rout);
      passed = 1'b0;
    end
    #2;
    if (out !== 1'b1) begin
      $display("FAILED: third delay, expected 1'b1, got %b", out);
      passed = 1'b0;
    end
    if (rout != 5.0) begin
      $display("FAILED: third delay (real), expected 5.0, got %f", rout);
      passed = 1'b0;
    end

    if (passed) $display("PASSED");

  end
endmodule
