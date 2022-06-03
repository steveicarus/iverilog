// Check that blocking partial writes to a 4-state vector array element are
// correctly handlded.

module test;

  reg failed = 1'b0;

  `define check(val, exp) \
    if (exp !== val) begin \
      $display("FAILED. Got %b, expected %b.", val, exp); \
      failed = 1'b1; \
    end

  reg [3:0] x[1:0];
  integer i;

  initial begin
    // Immediate index

    // Within bounds
    x[0] = 'hx;
    x[0][2:1] = 2'b10;
    `check(x[0], 4'bx10x)

    // Partially oob at high and low side
    x[0] = 'hx;
    x[0][4:-1] = 6'b101010;
    `check(x[0], 4'b0101)

    // Partially oob at low side
    x[0] = 'hx;
    x[0][0:-1] = 2'b10;
    `check(x[0], 4'bxxx1)

    // Partially oob at high side
    x[0] = 'hx;
    x[0][4:3] = 2'b01;
    `check(x[0], 4'b1xxx)

    // Fully oob at low side
    x[0] = 'hx;
    x[0][-1:-2] = 2'b11;
    `check(x[0], 4'bxxxx)

    // Fully oob at high side
    x[0] = 'hx;
    x[0][6:5] = 2'b11;
    `check(x[0], 4'bxxxx)

    // Variable index

    // Within bounds
    i = 1;
    x[0] = 'hx;
    x[0][i+:2] = 2'b10;
    `check(x[0], 4'bx10x)

    // Partially oob at high and low side
    i = -1;
    x[0] = 'hx;
    x[0][i+:6] = 6'b101010;
    `check(x[0], 4'b0101)

    // Partially oob at low side
    i = -1;
    x[0] = 'hx;
    x[0][i+:2] = 2'b10;
    `check(x[0], 4'bxxx1)

    // Partially oob at high side
    i = 3;
    x[0] = 'hx;
    x[0][i+:2] = 2'b01;
    `check(x[0], 4'b1xxx)

    // Fully oob at low side
    i = -2;
    x[0] = 'hx;
    x[0][i+:2] = 2'b11;
    `check(x[0], 4'bxxxx)

    // Fully oob at high side
    i = 5;
    x[0] = 'hx;
    x[0][i+:2] = 2'b11;
    `check(x[0], 4'bxxxx)

    // Undefined index
    i = 'hx;
    x[0] = 'hx;
    x[0][i+:2] = 2'b11;
    `check(x[0], 4'bxxxx)

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
