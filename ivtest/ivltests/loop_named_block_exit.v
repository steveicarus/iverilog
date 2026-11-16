module loop_named_block_exit;
  integer limit = 4;
  integer completed = 0;

  function automatic integer calculate(input integer stop);
    integer total;
    total = 0;
    for (integer i = 0; i < 6; i = i + 1) begin : body
      integer value;
      value = i;
      begin : inner
        if (i == 1) continue;
        if (i == stop) break;
        total = total + value;
      end
    end
    calculate = total;
  endfunction

  initial begin
    if (calculate(limit) != 5) $fatal(1, "function loop exit");
    completed = completed + 1;
  end

  final begin
    integer i;
    integer count;
    count = 0;
    for (i = 0; i < 6; i = i + 1) begin : body
      begin : inner
        if (i == 1) continue;
        if (i == 4) break;
        count = count + 1;
      end
    end
    if (count != 3 || i != 4 || completed != 1)
      $fatal(1, "final loop exit");
    $display("PASSED");
  end
endmodule
