module top;
  integer passed;
  integer idx;
  byte  res;
  string str = "hello";
 
  initial begin
    passed = 1'b1;

    idx = 0;
    str[idx] = "H";
    if (str != "Hello") begin
      $display("FAILED: Expected 'Hello' for index 0, got '%s'", str);
      passed = 1'b0;
    end
    res = str[idx];
    if (res != "H") begin
      $display("FAILED: Expected 'H' for index 0, got '%c'(%0d)", res, res);
      passed = 1'b0;
    end

    idx = -1;
    str[idx] = "N";
    if (str != "Hello") begin
      $display("FAILED: Expected 'Hello' for index -1, got '%s'", str);
      passed = 1'b0;
    end
    res = str[idx];
    if (res != 0) begin
      $display("FAILED: Expected 0 for index -1, got (%0d)", res);
      passed = 1'b0;
    end

    idx = 5;
    str[idx] = "!";
    if (str != "Hello") begin
      $display("FAILED: Expected 'Hello' for index 5, got '%s'", str);
      passed = 1'b0;
    end
    res = str[idx];
    if (res != 0) begin
      $display("FAILED: Expected 0 for index 5, got (%0d)", res);
      passed = 1'b0;
    end

    // We choose to treat an undefined index as out of range, but other
    // simulators treat it as a zero index.
    idx = 'x;
    str[idx] = "x";
    if ((str != "Hello") && (str != "xello")) begin
      $display("FAILED: Expected 'Hello/xello' for index x, got '%s'", str);
      passed = 1'b0;
    end
    res = str[idx];
    if ((res != 0) && (res != "x")) begin
      $display("FAILED: Expected 0/x for index x, got (%0d)", res);
      passed = 1'b0;
    end

    idx = 'z;
    str[idx] = "z";
    if ((str != "Hello") && (str != "zello")) begin
      $display("FAILED: Expected 'Hello/zello' for index z, got '%s'", str);
      passed = 1'b0;
    end
    res = str[idx];
    if ((res != 0) && (res != "z")) begin
      $display("FAILED: Expected 0/z for index z, got (%0d)", res);
      passed = 1'b0;
    end

    if (passed) $display("PASSED");
  end
endmodule
