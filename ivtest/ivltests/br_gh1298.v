module top;
  reg passed;
  string strt;
  string strf;
  string res;
  reg cond;

  initial begin
    passed = 1'b1;

    strt = "True string";
    strf = "False string";

    cond = 1'b1;
    res = cond ? strt : "False const string";
    if (res != strt) begin
      $display("Failed: expected the true string '%s', got '%s'", strt, res);
      passed = 1'b0;
    end

    cond = 1'b0;
    res = cond ? strt : "False const string";
    if (res != "False const string") begin
      $display("Failed: expected the false string 'False const string', got '%s'", res);
      passed = 1'b0;
    end

    cond = 1'bx;
    res = cond ? strt : "False const string";
    if (res != "") begin
      $display("Failed: expected the empty string '', got '%s'", res);
      passed = 1'b0;
    end

    cond = 1'b1;
    res = cond ? "True const string" : strf;
    if (res != "True const string") begin
      $display("Failed: expected the true string 'True const string', got '%s'", res);
      passed = 1'b0;
    end

    cond = 1'b0;
    res = cond ? "True const string" : strf;
    if (res != strf) begin
      $display("Failed: expected the false string '%s', got '%s'", strf, res);
      passed = 1'b0;
    end

    cond = 1'bx;
    res = cond ? "True const string" : strf;
    if (res != "") begin
      $display("Failed: expected the empty string '', got '%s'", res);
      passed = 1'b0;
    end

    cond = 1'b1;
    res = cond ? strt : strf;
    if (res != strt) begin
      $display("Failed: expected the true string '%s', got '%s'", strt, res);
      passed = 1'b0;
    end

    cond = 1'b0;
    res = cond ? strt : strf;
    if (res != strf) begin
      $display("Failed: expected the false string '%s', got '%s'", strf, res);
      passed = 1'b0;
    end

    cond = 1'bx;
    res = cond ? strt : strf;
    if (res != "") begin
      $display("Failed: expected the empty string '', got '%s'", res);
      passed = 1'b0;
    end

    strt = "matching string";
    strf = "matching string";
    res = cond ? strt : strf;
    if (res != "matching string") begin
      $display("Failed: expected the string 'matching string', got '%s'", res);
      passed = 1'b0;
    end

    res = cond ? strt : "matching string";
    if (res != "matching string") begin
      $display("Failed: expected the string 'matching string', got '%s'", res);
      passed = 1'b0;
    end

    res = cond ? "matching string" : strf;
    if (res != "matching string") begin
      $display("Failed: expected the string 'matching string', got '%s'", res);
      passed = 1'b0;
    end

    if  (passed) $display("PASSED");
  end
endmodule
