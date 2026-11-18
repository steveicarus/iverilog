// Check that runtime compound assignments perform four-state operations before
// converting the result to two-state.

module test;

  bit failed = 1'b0;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  bit [3:0] value;
  bit [7:0] array[0:1];
  bit [7:0] word;
  integer index;

  initial begin
    value = 5;
    value += 4'b000x;
    `check(value, 4'd0);
    value = 5;
    value <<= 4'b00x0;
    `check(value, 4'd0);

    word = 8'ha5;
    word[7:4] += 4'b000x;
    `check(word, 8'h05);
    array[1] = 8'ha5;
    index = 1;
    array[index] += 8'b0000000x;
    `check(array[1], 8'h00);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
