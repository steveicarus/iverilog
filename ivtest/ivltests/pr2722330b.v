module top;
  parameter py = 8'sb10101010 >> 3'sb110;
  parameter pz = 8'sb10101010 >>> 3'sb110;

  reg passed;
  reg signed [7:0] ry, rz, a;
  wire signed [7:0] wy, wz;

  assign wy = a >> 3'sb110;
  assign wz = a >>> 3'sb110;

  initial begin
    passed = 1'b1;
    // Example vector
    a = 8'sb10101010;
    #1;

    // Check the parameter results.
    if (py !== 8'b00000010) begin
      $display("Failed parameter >>, expected 8'b00000010, got %b", py);
      passed = 1'b0;
    end
    if (pz !== 8'b11111110) begin
      $display("Failed parameter >>>, expected 8'b11111110, got %b", pz);
      passed = 1'b0;
    end

    // Check the procedural results.
    ry = a >> 3'sb110;
    if (ry !== 8'b00000010) begin
      $display("Failed procedural >>, expected 8'b00000010, got %b", ry);
      passed = 1'b0;
    end
    rz = a >>> 3'sb110;
    if (rz !== 8'b11111110) begin
      $display("Failed procedural >>>, expected 8'b11111110, got %b", rz);
      passed = 1'b0;
    end

    // Check the CA results.
    if (wy !== 8'b00000010) begin
      $display("Failed CA >>, expected 8'b00000010, got %b", wy);
      passed = 1'b0;
    end
    if (wz !== 8'b11111110) begin
      $display("Failed CA >>>, expected 8'111111110, got %b", wz);
      passed = 1'b0;
    end

    if (passed) $display("PASSED");
  end
endmodule
