module top;
  reg pass;
  reg [3:0] value;
  reg [3:0] in;

  initial begin
    pass = 1'b1;
    value = 4'b1001;
    if (value !== 4'b1001) begin
      $display("Failed: initial value, expected 4'b1001, got %b", value);
      pass = 1'b0;
    end

    in = 4'bzx10;
    // This should work since it is really the whole value.
    force value[0 +: 4] = in;
    if (value !== 4'bzx10) begin
      $display("Failed: force value, expected 4'bzx10, got %b", value);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
