// Check that X/Z repeat counts skip the body in constant functions.

module test;

  function f_unsigned(input [64:0] count);
    begin : body
      f_unsigned = 1'b0;
      repeat (count) begin
        f_unsigned = 1'b1;
        disable body;
      end
    end
  endfunction

  function f_signed(input signed [64:0] count);
    begin : body
      f_signed = 1'b0;
      repeat (count) begin
        f_signed = 1'b1;
        disable body;
      end
    end
  endfunction

  localparam [1:0] ALL_X = {f_unsigned(65'bx), f_signed(65'bx)};
  localparam [1:0] ALL_Z = {f_unsigned(65'bz), f_signed(65'bz)};
  localparam [1:0] LOW_X = {f_unsigned(65'b1x), f_signed(65'b1x)};
  localparam [1:0] LOW_Z = {f_unsigned(65'b1z), f_signed(65'b1z)};
  localparam [1:0] HIGH_X = {f_unsigned({1'bx, 64'd1}),
                           f_signed({1'bx, 64'd1})};
  localparam [1:0] HIGH_Z = {f_unsigned({1'bz, 64'd1}),
                           f_signed({1'bz, 64'd1})};

  reg failed;

  `define check(val) \
    if (val !== 2'b00) begin \
      $display("FAILED(%0d): '%s' expected 00, got %b", \
               `__LINE__, `"val`", val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    `check(ALL_X);
    `check(ALL_Z);
    `check(LOW_X);
    `check(LOW_Z);
    `check(HIGH_X);
    `check(HIGH_Z);
    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
