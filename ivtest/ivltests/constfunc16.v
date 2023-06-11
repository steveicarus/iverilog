// Check that break and continue are supported in constant functions in for
// loops

module test;

  function automatic integer f1(integer x);
    integer j = 0;
    for (integer i = 0; i < 10; i++) begin
      if (i >= x) begin
        break;
      end
      j++;
    end
    return j;
  endfunction

  function automatic integer f2(integer x);
    integer j = 0;
    for (integer i = 0; i < x; i++) begin
      if (i % 2 == 0) begin
        continue;
      end
      j++;
    end
    return j;
  endfunction

  reg [f1(3):0] x;
  reg [f2(6):0] y;

  initial begin
    if ($bits(x) === 4 && $bits(y) === 4) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
