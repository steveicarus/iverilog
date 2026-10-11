// Test for GitHub issue #1574:
// $clog2 of a sized signed negative value must treat the argument as
// unsigned (IEEE 1800-2017 §20.8.1). The constant evaluator was
// incorrectly raising the result to integer_width (32) for all signed
// negative arguments, regardless of whether they had a defined length.
module br_gh1574;
  localparam signed [0:0] A = 1'sb1;    // unsigned value 1        -> clog2 = 0
  localparam signed [3:0] B = 4'sb1111; // unsigned value 15       -> clog2 = 4
  localparam signed [3:0] C = 4'sb1000; // unsigned value 8        -> clog2 = 3
  localparam              D = -1;        // unsized neg, implicit 32-bit -> clog2 = 32
  localparam signed [1:0] E = 2'sb10;   // unsigned value 2        -> clog2 = 1

  initial begin
    if ($clog2(A) !== 0) begin
      $display("FAILED: clog2(1'sb1) = %0d, expected 0", $clog2(A));
      $finish;
    end
    if ($clog2(B) !== 4) begin
      $display("FAILED: clog2(4'sb1111) = %0d, expected 4", $clog2(B));
      $finish;
    end
    if ($clog2(C) !== 3) begin
      $display("FAILED: clog2(4'sb1000) = %0d, expected 3", $clog2(C));
      $finish;
    end
    if ($clog2(D) !== 32) begin
      $display("FAILED: clog2(-1) = %0d, expected 32", $clog2(D));
      $finish;
    end
    if ($clog2(E) !== 1) begin
      $display("FAILED: clog2(2'sb10) = %0d, expected 1", $clog2(E));
      $finish;
    end
    $display("PASSED");
  end
endmodule
