// Test for GitHub issue #521
// Loop index should be allowed in outer dimension of multi-dimensional packed arrays
module test;
  logic [3:0][3:0] a;

  initial begin
    a = 0;
    for (int i=0; i<4; i++)
      a[i][3] = 1;

    // Each 4-bit sub-array has bit 3 set to 1, so each nibble is 0x8
    if (a !== 16'h8888) begin
      $display("FAILED: a = %h, expected 8888", a);
      $finish;
    end

    $display("PASSED");
  end
endmodule
