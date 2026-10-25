// Check that callback payloads contain the selected bits of their object.

module test;
  reg [64:0] bits;

  initial begin
    bits = 0;
    $watch_value(bits[3:0]);
    $watch_value(bits[64:61]);
    $watch_value(bits[64]);
    #1 bits = {4'ha, 57'b0, 4'h7};
    #1 bits = {4'h3, 57'b0, 4'h2};
    #1;
    if ($check_values(6)) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end
endmodule
