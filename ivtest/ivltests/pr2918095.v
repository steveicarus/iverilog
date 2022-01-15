module bug;
  reg pass;
  reg [7:0] a, b;
  real r;

  initial begin
    pass = 1'b1;
    a = 8'd255;
    b = 8'd255;
    if ((a + b) != 254.0) begin
      $display("FAILED: addition != real constant.");
      pass = 1'b0;
    end
    r = 254.0;
    if ((a + b) != r) begin
      $display("FAILED: addition != real variable.");
      pass = 1'b0;
    end

    if ((a * b) != 1.0) begin
      $display("FAILED: multiplication != real constant.");
      pass = 1'b0;
    end
    r = 1.0;
    if ((a * b) != r) begin
      $display("FAILED: multiplication != real variable.");
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
