module bug;
reg pass;
reg Select;
reg signed [3:0] Delta;
reg signed [5:0] Value;

wire signed [5:0] Value_ca = (Select ? 12 : 8) + Delta;

initial begin
  pass = 1'b1;
  Select = 1;
  Delta = -7;
  Value = (Select ? 12 : 8) + Delta;
  if (Value !== 5) begin
    $display("FAILED: procedural assign, expected 5, got %d", Value);
    pass = 1'b0;
  end

  #1;
  if (Value_ca !== 5) begin
    $display("FAILED: continuous assign, expected 5, got %d", Value_ca);
    pass = 1'b0;
  end

  if (pass) $display("PASSED");
end

endmodule
