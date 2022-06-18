// Check that non-blocking event controlled partial writes to a 4-state vector
// are correctly handlded.

module test;

  reg failed = 1'b0;

  `define check(val, exp) \
    if (exp !== val) begin \
      $display("FAILED. Got %b, expected %b.", val, exp); \
      failed = 1'b1; \
    end

  reg [3:0] x;
  integer i;

  event e;

  initial begin
    // Immediate index

    // Within bounds
    x = 'hx;
    x[2:1] <= @e 2'b10;
    -> e;
    `check(x, 4'bx10x)

    // Partially oob at high and low side
    x = 'hx;
    x[4:-1] <= @e 6'b101010;
    -> e;
    `check(x, 4'b0101)

    // Partially oob at low side
    x = 'hx;
    x[0:-1] <= @e 2'b10;
    -> e;
    `check(x, 4'bxxx1)

    // Partially oob at high side
    x = 'hx;
    x[4:3] <= @e 2'b01;
    -> e;
    `check(x, 4'b1xxx)

    // Fully oob at low side
    x = 'hx;
    x[-1:-2] <= @e 2'b11;
    -> e;
    `check(x, 4'bxxxx)

    // Fully oob at high side
    x = 'hx;
    x[6:5] <= @e 2'b11;
    -> e;
    `check(x, 4'bxxxx)

    // Variable index

    // Within bounds
    i = 1;
    x = 'hx;
    x[i+:2] <= @e 2'b10;
    -> e;
    `check(x, 4'bx10x)

    // Partially oob at high and low side
    i = -1;
    x = 'hx;
    x[i+:6] <= @e 6'b101010;
    -> e;
    `check(x, 4'b0101)

    // Partially oob at low side
    i = -1;
    x = 'hx;
    x[i+:2] <= @e 2'b10;
    -> e;
    `check(x, 4'bxxx1)

    // Partially oob at high side
    i = 3;
    x = 'hx;
    x[i+:2] <= @e 2'b01;
    -> e;
    `check(x, 4'b1xxx)

    // Fully oob at low side
    i = -2;
    x = 'hx;
    x[i+:2] <= @e 2'b11;
    -> e;
    `check(x, 4'bxxxx)

    // Fully oob at high side
    i = 5;
    x = 'hx;
    x[i+:2] <= @e 2'b11;
    -> e;
    `check(x, 4'bxxxx)

    // Undefined index
    i = 'hx;
    x = 'hx;
    x[i+:2] <= @e 2'b11;
    -> e;
    `check(x, 4'bxxxx)

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
