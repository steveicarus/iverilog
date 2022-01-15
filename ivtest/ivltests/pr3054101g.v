// Check the various variable array selects (small to large).
module top;
  reg passed;

  wire [1:0] a [1:4];
  wire [0:0] s0 = 0;
  wire [1:0] s1 = 0;
  wire [2:0] s2 = 0;
  reg [1:0] ar [1:4];

  wire [1:0] c [-3:0];
  wire [0:0] s3 = 0;
  wire [1:0] s4 = 0;
  reg [1:0] cr [-3:0];

  wire [1:0] res_a0 = a[s0];
  wire [1:0] res_a1 = a[s1];
  wire [1:0] res_a2 = a[s2];

  wire [1:0] res_c3 = c[s3];
  wire [1:0] res_c4 = c[s4];

  reg res_a [1:4];
  reg res_c [-3:0];

  assign a[1] = 2'd0;
  assign a[2] = 2'b1;
  assign a[3] = 2'd2;
  assign a[4] = 2'd3;

  assign c[-3] = 2'd0;
  assign c[-2] = 2'b1;
  assign c[-1] = 2'd2;
  assign c[0] = 2'd3;

  initial begin
    #1;
    passed = 1'b1;

    ar[1] = 2'd0;
    ar[2] = 2'b1;
    ar[3] = 2'd2;
    ar[4] = 2'd3;

    cr[-3] = 2'd0;
    cr[-2] = 2'b1;
    cr[-1] = 2'd2;
    cr[0] = 2'd3;

    // Check procedural R-value variable bit selects of a net.

    $display("a[s0]: %b", a[s0]);
    if (a[s0] !== 2'bxx) begin
      $display("Failed a[s0], expected 2'bxx, got %b", a[s0]);
      passed = 1'b0;
    end

    $display("a[s1]: %b", a[s1]);
    if (a[s1] !== 2'bxx) begin
      $display("Failed a[s1], expected 2'bxx, got %b", a[s1]);
      passed = 1'b0;
    end

    $display("a[s2]: %b", a[s2]);
    if (a[s2] !== 2'bxx) begin
      $display("Failed a[s2], expected 2'bxx, got %b", a[s2]);
      passed = 1'b0;
    end

    $display("c[s3]: %b", c[s3]);
    if (c[s3] !== 2'b11) begin
      $display("Failed c[s3], expected 2'b11, got %b", c[s3]);
      passed = 1'b0;
    end

    $display("c[s4]: %b", c[s4]);
    if (c[s4] !== 2'b11) begin
      $display("Failed c[s4], expected 2'b11, got %b", c[s4]);
      passed = 1'b0;
    end

    // Check procedural R-value variable bit selects of a reg.

    $display("ar[s0]: %b", ar[s0]);
    if (ar[s0] !== 2'bxx) begin
      $display("Failed ar[s0], expected 2'bxx, got %b", ar[s0]);
      passed = 1'b0;
    end

    $display("ar[s1]: %b", ar[s1]);
    if (ar[s1] !== 2'bxx) begin
      $display("Failed ar[s1], expected 2'bxx, got %b", ar[s1]);
      passed = 1'b0;
    end

    $display("ar[s2]: %b", ar[s2]);
    if (ar[s2] !== 2'bxx) begin
      $display("Failed ar[s2], expected 2'bxx, got %b", ar[s2]);
      passed = 1'b0;
    end

    $display("cr[s3]: %b", cr[s3]);
    if (cr[s3] !== 2'b11) begin
      $display("Failed cr[s3], expected 2'b11, got %b", cr[s3]);
      passed = 1'b0;
    end

    $display("cr[s4]: %b", cr[s4]);
    if (cr[s4] !== 2'b11) begin
      $display("Failed cr[s4], expected 2'b11, got %b", cr[s4]);
      passed = 1'b0;
    end

    // Check continuous assignment R-value variable bit selects.

    if (res_a0 !== 2'bxx) begin
      $display("Failed res_a0, expected 2'bxx, got %b", res_a0);
      passed = 1'b0;
    end

    if (res_a1 !== 2'bxx) begin
      $display("Failed res_a1, expected 2'bxx, got %b", res_a1);
      passed = 1'b0;
    end

    if (res_a2 !== 2'bxx) begin
      $display("Failed res_a2, expected 2'bxx, got %b", res_a2);
      passed = 1'b0;
    end

    if (res_c3 !== 2'b11) begin
      $display("Failed res_c3, expected 2'b11, got %b", res_c3);
      passed = 1'b0;
    end

    if (res_c4 !== 2'b11) begin
      $display("Failed res_c4, expected 2'b11, got %b", res_c4);
      passed = 1'b0;
    end

    // Check procedural L-value variable bit selects.

    res_a[1] = 1'bx;
    res_a[2] = 1'bx;
    res_a[3] = 1'bx;
    res_a[4] = 1'bx;
    res_a[s0] = 1'b0;
    if (res_a[1] !== 1'bx) begin
      $display("Failed res_a[s0], expected 1'bx for [1], got %b", res_a[1]);
      passed = 1'b0;
    end
    if (res_a[2] !== 1'bx) begin
      $display("Failed res_a[s0], expected 1'bx for [2], got %b", res_a[2]);
      passed = 1'b0;
    end
    if (res_a[3] !== 1'bx) begin
      $display("Failed res_a[s0], expected 1'bx for [3], got %b", res_a[3]);
      passed = 1'b0;
    end
    if (res_a[4] !== 1'bx) begin
      $display("Failed res_a[s0], expected 1'bx for [4], got %b", res_a[4]);
      passed = 1'b0;
    end

    res_a[1] = 1'bx;
    res_a[2] = 1'bx;
    res_a[3] = 1'bx;
    res_a[4] = 1'bx;
    res_a[s1] = 1'b0;
    if (res_a[1] !== 1'bx) begin
      $display("Failed res_a[s1], expected 1'bx for [1], got %b", res_a[1]);
      passed = 1'b0;
    end
    if (res_a[2] !== 1'bx) begin
      $display("Failed res_a[s1], expected 1'bx for [2], got %b", res_a[2]);
      passed = 1'b0;
    end
    if (res_a[3] !== 1'bx) begin
      $display("Failed res_a[s1], expected 1'bx for [3], got %b", res_a[3]);
      passed = 1'b0;
    end
    if (res_a[4] !== 1'bx) begin
      $display("Failed res_a[s1], expected 1'bx for [4], got %b", res_a[4]);
      passed = 1'b0;
    end

    res_a[1] = 1'bx;
    res_a[2] = 1'bx;
    res_a[3] = 1'bx;
    res_a[4] = 1'bx;
    res_a[s2] = 1'b0;
    if (res_a[1] !== 1'bx) begin
      $display("Failed res_a[s2], expected 1'bx for [1], got %b", res_a[1]);
      passed = 1'b0;
    end
    if (res_a[2] !== 1'bx) begin
      $display("Failed res_a[s2], expected 1'bx for [2], got %b", res_a[2]);
      passed = 1'b0;
    end
    if (res_a[3] !== 1'bx) begin
      $display("Failed res_a[s2], expected 1'bx for [3], got %b", res_a[3]);
      passed = 1'b0;
    end
    if (res_a[4] !== 1'bx) begin
      $display("Failed res_a[s2], expected 1'bx for [4], got %b", res_a[4]);
      passed = 1'b0;
    end

    res_c[-3] = 1'bx;
    res_c[-2] = 1'bx;
    res_c[-1] = 1'bx;
    res_c[0] = 1'bx;
    res_c[s3] = 1'b0;
    if (res_c[-3] !== 1'bx) begin
      $display("Failed res_c[s3], expected 1'bx for [-3], got %b", res_c[-3]);
      passed = 1'b0;
    end
    if (res_c[-2] !== 1'bx) begin
      $display("Failed res_c[s3], expected 1'bx for [-2], got %b", res_c[-2]);
      passed = 1'b0;
    end
    if (res_c[-1] !== 1'bx) begin
      $display("Failed res_c[s3], expected 1'bx for [-1], got %b", res_c[-1]);
      passed = 1'b0;
    end
    if (res_c[0] !== 1'b0) begin
      $display("Failed res_c[s3], expected 1'b0 for [0], got %b", res_c[0]);
      passed = 1'b0;
    end

    res_c[-3] = 1'bx;
    res_c[-2] = 1'bx;
    res_c[-1] = 1'bx;
    res_c[0] = 1'bx;
    res_c[s4] = 1'b0;
    if (res_c[-3] !== 1'bx) begin
      $display("Failed res_c[s4], expected 1'bx for [-3], got %b", res_c[-3]);
      passed = 1'b0;
    end
    if (res_c[-2] !== 1'bx) begin
      $display("Failed res_c[s4], expected 1'bx for [-2], got %b", res_c[-2]);
      passed = 1'b0;
    end
    if (res_c[-1] !== 1'bx) begin
      $display("Failed res_c[s4], expected 1'bx for [-1], got %b", res_c[-1]);
      passed = 1'b0;
    end
    if (res_c[0] !== 1'b0) begin
      $display("Failed res_c[s4], expected 1'b0 for [0], got %b", res_c[0]);
      passed = 1'b0;
    end

    if (passed) $display("Compare tests passed");
  end
endmodule
