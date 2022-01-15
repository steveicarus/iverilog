// Check that the >> and >>> operators with unsigned values.
module top;
  parameter py = 8'b10101010 >> 3'b101;
  parameter pz = 8'b10101010 >>> 3'b101;

  reg passed;
  reg  [7:0] a;
  reg  [2:0] b;
  wire [7:0] wy, wz;
  reg  [7:0] ry, rz;

  // Check CA code.
  assign wy = a >>  b;
  assign wz = a >>> b;

  initial begin
    passed = 1'b1;
    // Example vector
    a = 8'b10101010;
    b = 3'b101;
    #1;

    // Check the parameter results.
    if (py !== 8'b00000101) begin
      $display("Failed param. >>, expected 8'b00000101, got %b", py);
      passed = 1'b0;
    end
    if (pz !== 8'b00000101) begin
      $display("Failed param. >>>, expected 8'b00000101, got %b", pz);
      passed = 1'b0;
    end

    // Check the procedural results.
    ry = a >> b;
    if (ry !== 8'b00000101) begin
      $display("Failed procedural >>, expected 8'b00000101, got %b", ry);
      passed = 1'b0;
    end
    rz = a >>> b;
    if (rz !== 8'b00000101) begin
      $display("Failed procedural >>>, expected 8'b00000101, got %b", rz);
      passed = 1'b0;
    end

    // Check the CA results.
    if (wy !== 8'b00000101) begin
      $display("Failed CA >>, expected 8'b00000101, got %b", wy);
      passed = 1'b0;
    end
    if (wz !== 8'b00000101) begin
      $display("Failed CA >>>, expected 8'b00000101, got %b", wz);
      passed = 1'b0;
    end

    if (passed) $display("PASSED");
  end
endmodule
