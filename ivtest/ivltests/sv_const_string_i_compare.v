module top;
  parameter string str1 = "Hello";
  parameter string str2 = "World";
  parameter string str3 = "hello";
  parameter int res1 = str1.compare(str2);
  parameter int res2 = str2.compare(str1);
  parameter int res3 = str1.compare(str1);
  parameter int res4 = str2.compare("World");
  parameter int ires1 = str1.icompare(str2);
  parameter int ires2 = str2.icompare(str1);
  parameter int ires3 = str1.icompare(str3);
  parameter int ires4 = str2.icompare("world");
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
      $display("FAILED: expected str1.compare(str2) -> -1, got %0d", ires1);
      passed = 1'b0;
    end

    if (ires2 <= 0) begin
      $display("FAILED: expected str2.compare(str1) -> 1, got %0d", ires2);
      passed = 1'b0;
    end

    if (ires3 !== 0) begin
      $display("FAILED: expected str1.compare(str3) -> 0, got %0d", ires3);
      passed = 1'b0;
    end

    if (ires4 !== 0) begin
      $display("FAILED: expected str2.compare(\"world\") -> 0, got %0d", ires4);
      passed = 1'b0;
    end

    if (passed) $display("PASSED");
  end
endmodule
