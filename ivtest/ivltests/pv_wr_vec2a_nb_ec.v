// Check that non-blocking event controlled partial writes to a 2-state vector
// array element are correctly handlded.

module test;

  reg failed = 1'b0;

  `define check(val, exp) \
    if (exp !== val) begin \
      $display("FAILED. Got %b, expected %b.", val, exp); \
      failed = 1'b1; \
    end

  bit [3:0] x[1:0];
  integer i;

  event e;

  initial begin
    // Immediate index

    // Within bounds
    x[0] = 'h0;
    x[0][2:1] <= @e 2'b10;
    -> e;
    `check(x[0], 4'b0100)

    // Partially oob at high and low side
    x[0] = 'h0;
    x[0][4:-1] <= @e 6'b101010;
    -> e;
    `check(x[0], 4'b0101)

    // Partially oob at low side
    x[0] = 'h0;
    x[0][0:-1] <= @e 2'b10;
    -> e;
    `check(x[0], 4'b0001)

    // Partially oob at high side
    x[0] = 'h0;
    x[0][4:3] <= @e 2'b01;
    -> e;
    `check(x[0], 4'b1000)

    // Fully oob at low side
    x[0] = 'h0;
    x[0][-1:-2] <= @e 2'b11;
    -> e;
    `check(x[0], 4'b0000)

    // Fully oob at high side
    x[0] = 'h0;
    x[0][6:5] <= @e 2'b11;
    -> e;
    `check(x[0], 4'b0000)

    // Variable index

    // Within bounds
    i = 1;
    x[0] = 'h0;
    x[0][i+:2] <= @e 2'b10;
    -> e;
    `check(x[0], 4'b0100)

    // Partially oob at high and low side
    i = -1;
    x[0] = 'h0;
    x[0][i+:6] <= @e 6'b101010;
    -> e;
    `check(x[0], 4'b0101)

    // Partially oob at low side
    i = -1;
    x[0] = 'h0;
    x[0][i+:2] <= @e 2'b10;
    -> e;
    `check(x[0], 4'b0001)

    // Partially oob at high side
    i = 3;
    x[0] = 'h0;
    x[0][i+:2] <= @e 2'b01;
    -> e;
    `check(x[0], 4'b1000)

    // Fully oob at low side
    i = -2;
    x[0] = 'h0;
    x[0][i+:2] <= @e 2'b11;
    -> e;
    `check(x[0], 4'b0000)

    // Fully oob at high side
    i = 5;
    x[0] = 'h0;
    x[0][i+:2] <= @e 2'b11;
    -> e;
    `check(x[0], 4'b0000)

    // Undefined index
    i = 'hx;
    x[0] = 'h0;
    x[0][i+:2] <= @e 2'b11;
    -> e;
    `check(x[0], 4'b0000)

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
