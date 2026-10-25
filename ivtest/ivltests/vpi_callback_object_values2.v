// Check that callback payloads preserve the signedness of their object.

module test;
  reg signed [7:0] value;

  initial begin
    value = 0;
    $watch_value(value);
    #1 value = -1;
    #1 value = -8;
    #1;
    if ($check_values(2)) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end
endmodule
