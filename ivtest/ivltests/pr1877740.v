module top;
  reg pass = 1;
  initial begin
    if (4'sd2 < -2) begin
      $display("Failed for operator <");
      pass = 0;
    end
    if (4'sd2 <= -2) begin
      $display("Failed for operator <=");
      pass = 0;
    end
    if (-2 > 4'sd2) begin
      $display("Failed for operator >");
      pass = 0;
    end
    if (-2 >= 4'sd2) begin
      $display("Failed for operator >=");
      pass = 0;
    end
    if (pass) $display("PASSED");
  end
endmodule
