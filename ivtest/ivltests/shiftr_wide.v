module shiftr_wide;
  reg [129:0] value;
  reg signed [129:0] signed_one;
  reg signed [129:0] signed_x;
  reg signed [129:0] signed_z;
  reg [127:0] aligned_value;
  reg signed [127:0] aligned_one;
  reg signed [127:0] aligned_x;
  reg signed [127:0] aligned_z;
  integer amount;
  integer failed;

  function [129:0] reference_shift;
    input [129:0] arg;
    input integer shift;
    input pad;
    integer idx;
    begin
      for (idx = 0; idx < 130; idx = idx + 1) begin
        if (idx + shift < 130)
          reference_shift[idx] = arg[idx + shift];
        else
          reference_shift[idx] = pad;
      end
    end
  endfunction

  function [127:0] aligned_reference_shift;
    input [127:0] arg;
    input integer shift;
    input pad;
    integer idx;
    begin
      for (idx = 0; idx < 128; idx = idx + 1) begin
        if (idx + shift < 128)
          aligned_reference_shift[idx] = arg[idx + shift];
        else
          aligned_reference_shift[idx] = pad;
      end
    end
  endfunction

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

  task aligned_check;
    input [127:0] actual;
    input [127:0] expected;
    input integer test_id;
    begin
      if (actual !== expected) begin
        $display("FAILED aligned test %0d", test_id);
        failed = failed + 1;
      end
    end
  endtask

  initial begin
    value = {2'b1x, 32'h01234567, 32'h89abcdef, 32'hfedcba98,
             16'h7654, 8'b10xz01zx, 8'h69};
    signed_one = {1'b1, value[128:0]};
    signed_x = {1'bx, value[128:0]};
    signed_z = {1'bz, value[128:0]};
    aligned_value = {32'h01234567, 32'h89abcdef, 32'hfedcba98,
                     16'h7654, 8'b10xz01zx, 8'h69};
    aligned_one = {1'b1, aligned_value[126:0]};
    aligned_x = {1'bx, aligned_value[126:0]};
    aligned_z = {1'bz, aligned_value[126:0]};
    failed = 0;

    for (amount = 0; amount <= 131; amount = amount + 1) begin
      check(value >> amount,
            reference_shift(value, amount, 1'b0), amount * 4);
      check(signed_one >>> amount,
            reference_shift(signed_one, amount, signed_one[129]),
            amount * 4 + 1);
      check(signed_x >>> amount,
            reference_shift(signed_x, amount, signed_x[129]),
            amount * 4 + 2);
      check(signed_z >>> amount,
            reference_shift(signed_z, amount, signed_z[129]),
            amount * 4 + 3);
      aligned_check(aligned_value >> amount,
                    aligned_reference_shift(aligned_value, amount, 1'b0),
                    amount * 4);
      aligned_check(aligned_one >>> amount,
                    aligned_reference_shift(aligned_one, amount,
                                            aligned_one[127]),
                    amount * 4 + 1);
      aligned_check(aligned_x >>> amount,
                    aligned_reference_shift(aligned_x, amount,
                                            aligned_x[127]),
                    amount * 4 + 2);
      aligned_check(aligned_z >>> amount,
                    aligned_reference_shift(aligned_z, amount,
                                            aligned_z[127]),
                    amount * 4 + 3);
    end

    if (failed == 0)
      $display("PASSED");
  end
endmodule
