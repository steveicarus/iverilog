module top;
  string str1 = "Hello";
  string str2 = "World";
  string str3 = "hello";
  int res1 = str1.compare(str2);
  int res2 = str2.compare(str1);
  int res3 = str1.compare(str1);
  int res4 = str2.compare("World");
  int ires1 = str1.icompare(str2);
  int ires2 = str2.icompare(str1);
  int ires3 = str1.icompare(str3);
  int ires4 = str2.icompare("world");
  logic passed;

  initial begin
    passed = 1'b1;

    if (res1 >= 0) begin
      $display("FAILED: expected str1.compare(str2) -> -1, got %0d", res1);
      passed = 1'b0;
    end

    if (res2 <= 0) begin
      $display("FAILED: expected str2.compare(str1) -> 1, got %0d", res2);
      passed = 1'b0;
    end

    if (res3 !== 0) begin
      $display("FAILED: expected str1.compare(str1) -> 0, got %0d", res3);
      passed = 1'b0;
    end

    if (res4 !== 0) begin
      $display("FAILED: expected str2.compare(\"World\") -> 0, got %0d", res4);
      passed = 1'b0;
    end

    if (ires1 >= 0) begin
      $display("FAILED: expected str1.icompare(str2) -> -1, got %0d", ires1);
      passed = 1'b0;
    end

    if (ires2 <= 0) begin
      $display("FAILED: expected str2.icompare(str1) -> 1, got %0d", ires2);
      passed = 1'b0;
    end

    if (ires3 !== 0) begin
      $display("FAILED: expected str1.icompare(str3) -> 0, got %0d", ires3);
      passed = 1'b0;
    end

    if (ires4 !== 0) begin
      $display("FAILED: expected str2.icompare(\"world\") -> 0, got %0d", ires4);
      passed = 1'b0;
    end

    if (passed) $display("PASSED");
  end
endmodule
