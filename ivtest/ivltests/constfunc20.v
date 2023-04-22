// Check that break and continue are supported in constant functions in forever
// loops

module test;

  function automatic integer f(integer x);
    integer j = 0;
    integer i = 0;
    forever begin
      i++;
      if (i == x) begin
        break;
      end
      if (i % 2 == 0) begin
        continue;
      end
      j++;
    end
    return j;
  endfunction

  reg [f(10):0] x;

  initial begin
    if ($bits(x) === 6) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
