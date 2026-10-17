// Check that nonblocking event controls retain all 64 bits of the repeat count.

module test;

  reg [63:0] count;
  reg [7:0] vector_value;
  reg [7:0] vector_part;
  real real_value;
  reg [7:0] vector_array[0:0];
  reg failed;
  integer i, tick_number, expected;
  event tick;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  `define check_real(val, exp) \
    if (val != exp) begin \
      $display("FAILED(%0d). '%s' expected %f, got %f", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    for (i = 0; i < 7; i = i + 1) begin
      case (i)
        0: count = 0;
        1: count = 1;
        2: count = 2;
        3: count = 64'h100000000;
        4: count = 64'h100000001;
        5: count = 64'h8000000000000001;
        6: count = 64'hffffffffffffffff;
      endcase
      vector_value = 0;
      vector_part = 0;
      real_value = 0;
      vector_array[0] = 0;
      vector_value <= repeat(count) @(tick) 7;
      vector_part[3:0] <= repeat(count) @(tick) 7;
      real_value <= repeat(count) @(tick) 7.0;
      vector_array[0] <= repeat(count) @(tick) 7;
      for (tick_number = 0; tick_number < 3; tick_number = tick_number + 1) begin
        #1;
        expected = count <= tick_number ? 7 : 0;
        `check(vector_value, expected);
        `check(vector_part, expected);
        `check_real(real_value, expected);
        `check(vector_array[0], expected);
        ->tick;
      end
    end
    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
