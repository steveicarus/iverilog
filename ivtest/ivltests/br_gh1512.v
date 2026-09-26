module top;
  logic passed;
  logic [5:0] w = 5'b10110;
  logic [2:0] x = 3'b010;
  wire [3:0] a [1:5];
  assign a = '{4'hf, w, 3'b101, x, 5'b11001};

  initial begin
    passed = 1'b1;
    #1;
    if (a[1] !== 4'b1111) begin
      $display("FAILED: expected a[1] to be 4'b1111, got %b", a[1]);
      passed = 1'b0;
    end
    if (a[2] !== 4'b0110) begin
      $display("FAILED: expected a[2] to be 4'b0110, got %b", a[2]);
      passed = 1'b0;
    end
    if (a[3] !== 4'b0101) begin
      $display("FAILED: expected a[3] to be 4'b0101, got %b", a[3]);
      passed = 1'b0;
    end
    if (a[4] !== 4'b0010) begin
      $display("FAILED: expected a[4] to be 4'b0010, got %b", a[4]);
      passed = 1'b0;
    end
    if (a[5] !== 4'b1001) begin
      $display("FAILED: expected a[5] to be 4'b1001, got %b", a[5]);
      passed = 1'b0;
    end

    if (passed)  $display("PASSED");
  end
endmodule
