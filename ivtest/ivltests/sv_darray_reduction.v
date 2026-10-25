module top;
  logic passed;
  string sa [];
  real ra [];
  int ia [];
  string sres;
  real rres;
  int res;

  initial begin
    passed = 1'b1;
    sa = '{"Hello", "World", "!"};
    ra = '{1.0, 2.0, 4.0, 8.0, 1.0};
    ia = '{1,   3,   5,   9};

    res = $size(sa);
    if (res != 3) begin
      $display("FAILED: expected string array to have 3 elements, found %0d", res);
      passed = 1'b0;
    end

    res = $size(ra);
    if (res != 5) begin
      $display("FAILED: expected real array to have 5 elements, found %0d", res);
      passed = 1'b0;
    end

    rres = ra.product();
    if (rres != 64.0) begin
      $display("FAILED: expected real array 'product' to be 64.0, got %0d", rres);
      passed = 1'b0;
    end

    rres = ra.product;
    if (rres != 64.0) begin
      $display("FAILED: expected real array 'product (NP)' to be 64.0, got %0d", rres);
      passed = 1'b0;
    end

    rres = ra.sum();
    if (rres != 16.0) begin
      $display("FAILED: expected real array 'sum' to be 16.0, got %0d", rres);
      passed = 1'b0;
    end

    rres = ra.sum;
    if (rres != 16.0) begin
      $display("FAILED: expected real array 'sum (NP)' to be 16.0, got %0d", rres);
      passed = 1'b0;
    end

    res = $size(ia);
    if (res != 4) begin
      $display("FAILED: expected int array to have 4 elements, found %0d", res);
      passed = 1'b0;
    end

    res = ia.product();
    if (res != 135) begin
      $display("FAILED: expected int array 'product' to be 135, got %0d", res);
      passed = 1'b0;
    end

    res = ia.product;
    if (res != 135) begin
      $display("FAILED: expected int array 'product (NP)' to be 135, got %0d", res);
      passed = 1'b0;
    end

    res = ia.sum();
    if (res != 18) begin
      $display("FAILED: expected int array 'sum' to be 18, got %0d", res);
      passed = 1'b0;
    end

    res = ia.sum;
    if (res != 18) begin
      $display("FAILED: expected int array 'sum (NP)' to be 18, got %0d", res);
      passed = 1'b0;
    end

    res = ia.and();
    if (res != 1) begin
      $display("FAILED: expected int array 'and' to be 1, got %0d", res);
      passed = 1'b0;
    end

    res = ia.and;
    if (res != 1) begin
      $display("FAILED: expected int array 'and (NP)' to be 1, got %0d", res);
      passed = 1'b0;
    end

    res = ia.or();
    if (res != 15) begin
      $display("FAILED: expected int array 'or' to be 15, got %0d", res);
      passed = 1'b0;
    end

    res = ia.or;
    if (res != 15) begin
      $display("FAILED: expected int array 'or (NP)' to be 15, got %0d", res);
      passed = 1'b0;
    end

    res = ia.xor();
    if (res != 14) begin
      $display("FAILED: expected int array 'xor' to be 14, got %0d", res);
      passed = 1'b0;
    end

    res = ia.xor;
    if (res != 14) begin
      $display("FAILED: expected int array 'xor (NP)' to be 14, got %0d", res);
      passed = 1'b0;
    end

    if (passed) $display("PASSED");
  end
endmodule
