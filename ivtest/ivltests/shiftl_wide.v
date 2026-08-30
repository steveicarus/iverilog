module shiftl_wide;
  reg [63:0] narrow;
  reg [127:0] exact;
  reg [129:0] value;
  integer amount;
  integer failed;

  function [63:0] reference_narrow;
    input [63:0] arg;
    input integer shift;
    integer idx;
    begin
      for (idx = 0; idx < 64; idx = idx + 1) begin
        if (idx >= shift)
          reference_narrow[idx] = arg[idx - shift];
        else
          reference_narrow[idx] = 1'b0;
      end
    end
  endfunction

  function [127:0] reference_exact;
    input [127:0] arg;
    input integer shift;
    integer idx;
    begin
      for (idx = 0; idx < 128; idx = idx + 1) begin
        if (idx >= shift)
          reference_exact[idx] = arg[idx - shift];
        else
          reference_exact[idx] = 1'b0;
      end
    end
  endfunction

  function [129:0] reference_shift;
    input [129:0] arg;
    input integer shift;
    integer idx;
    begin
      for (idx = 0; idx < 130; idx = idx + 1) begin
        if (idx >= shift)
          reference_shift[idx] = arg[idx - shift];
        else
          reference_shift[idx] = 1'b0;
      end
    end
  endfunction

  task check_narrow;
    input [63:0] actual;
    input [63:0] expected;
    input integer test_id;
    begin
      if (actual !== expected) begin
        $display("FAILED narrow test %0d", test_id);
        failed = failed + 1;
      end
    end
  endtask

  task check_exact;
    input [127:0] actual;
    input [127:0] expected;
    input integer test_id;
    begin
      if (actual !== expected) begin
        $display("FAILED exact test %0d", test_id);
        failed = failed + 1;
      end
    end
  endtask

  task check;
    input [129:0] actual;
    input [129:0] expected;
    input integer test_id;
    begin
      if (actual !== expected) begin
        $display("FAILED test %0d", test_id);
        failed = failed + 1;
      end
    end
  endtask

  initial begin
    narrow = 64'h81xz_4567_89ab_cdef;
    exact = {64'h01234567_89abcdef, 64'hfedcba98_7654xz10};
    value = {2'b1x, 32'h01234567, 32'h89abcdef, 32'hfedcba98,
             16'h7654, 8'b10xz01zx, 8'h69};
    failed = 0;

    for (amount = 0; amount <= 65; amount = amount + 1) begin
      check_narrow(narrow << amount, reference_narrow(narrow, amount),
                   amount);
    end

    for (amount = 0; amount <= 129; amount = amount + 1) begin
      check_exact(exact << amount, reference_exact(exact, amount), amount);
    end

    for (amount = 0; amount <= 131; amount = amount + 1) begin
      check(value << amount, reference_shift(value, amount), amount);
    end

    if (failed == 0)
      $display("PASSED");
  end
endmodule
