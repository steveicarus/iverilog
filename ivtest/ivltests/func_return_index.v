// Check that a function return value can be used as an array or bit index.

module test;

  reg failed;
  integer index;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %h, got %h", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  function integer read_word(input integer sel);
    reg [31:0] values[0:1];
    begin
      values[0] = 10;
      values[1] = 20;
      read_word = sel;
      read_word = values[read_word];
    end
  endfunction

  function integer write_bit(input integer sel);
    reg [7:0] value;
    begin
      value = 0;
      write_bit = sel;
      value[write_bit] = 1'b1;
      write_bit = value;
    end
  endfunction

  initial begin
    failed = 1'b0;
    index = 0;
    `check(read_word(index), 10);
    `check(write_bit(index), 1);
    index = 1;
    `check(read_word(index), 20);
    `check(write_bit(index), 2);
    index = -1;
    `check(read_word(index), 32'bx);
    `check(write_bit(index), 0);
    index = 8;
    `check(read_word(index), 32'bx);
    `check(write_bit(index), 0);
    index = 'bx;
    `check(read_word(index), 32'bx);
    `check(write_bit(index), 0);
    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
