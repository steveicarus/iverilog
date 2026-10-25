// Check that callback payloads preserve real values.

module test;
  real value;

  initial begin
    value = 0;
    $watch_value(value);
    #1 value = 1.25;
    #1 value = -2.5;
    #1;
    if ($check_values(2)) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end
endmodule
