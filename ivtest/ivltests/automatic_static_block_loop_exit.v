module automatic_static_block_loop_exit;
  integer i;
  integer sum = 0;
  integer completed = 0;

  initial begin
    for (i = 0; i < 6; i = i + 1) begin
      automatic int value = i;
      begin : nested
        if (i == 1) continue;
        if (i == 4) break;
        sum = sum + value;
      end
    end
    if (i != 4 || sum != 5) $fatal(1, "for loop: i=%0d sum=%0d", i, sum);
    i = 0;
    repeat (6) begin
      automatic int value = i;
      i = i + 1;
      if (value == 1) continue;
      if (value == 3) break;
    end
    if (i != 4) $fatal(1, "repeat loop: i=%0d", i);
    i = 0;
    while (i < 6) begin
      automatic int value = i;
      i = i + 1;
      if (value == 1) continue;
      if (value == 3) break;
    end
    if (i != 4) $fatal(1, "while loop: i=%0d", i);
    i = 0;
    do begin
      automatic int value = i;
      i = i + 1;
      if (value == 1) continue;
      if (value == 3) break;
    end while (i < 6);
    if (i != 4) $fatal(1, "do loop: i=%0d", i);
    i = 0;
    forever begin
      automatic int value = i;
      i = i + 1;
      if (value == 1) continue;
      if (value == 3) break;
    end
    if (i != 4) $fatal(1, "forever loop: i=%0d", i);
    completed = completed + 1;
  end

  final begin
    if (completed != 1) $fatal(1, "post-loop code ran %0d times", completed);
    $display("PASSED");
  end
endmodule
