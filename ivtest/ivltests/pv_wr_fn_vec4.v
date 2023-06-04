// Check that blocking partial writes to a 4-state vector function return value
// are correctly handled.

module test;

  reg failed = 1'b0;

  `define check(val, exp) \
    if (exp !== val) begin \
      $display("FAILED. Got %b, expected %b.", val, exp); \
      failed = 1'b1; \
    end

  integer i;

  function reg [3:0] f(input reg unused);
  begin
    // Immediate index

    // Within bounds
    f = 'hx;
    f[2:1] = 2'b10;
    `check(f, 4'bx10x)

    // Partially oob at high and low side
    f = 'hx;
    f[4:-1] = 6'b101010;
    `check(f, 4'b0101)

    // Partially oob at low side
    f = 'hx;
    f[0:-1] = 2'b10;
    `check(f, 4'bxxx1)

    // Partially oob at high side
    f = 'hx;
    f[4:3] = 2'b01;
    `check(f, 4'b1xxx)

    // Fully oob at low side
    f = 'hx;
    f[-1:-2] = 2'b11;
    `check(f, 4'bxxxx)

    // Fully oob at high side
    f = 'hx;
    f[6:5] = 2'b11;
    `check(f, 4'bxxxx)

    // Variable index

    // Within bounds
    i = 1;
    f = 'hx;
    f[i+:2] = 2'b10;
    `check(f, 4'bx10x)

    // Partially oob at high and low side
    i = -1;
    f = 'hx;
    f[i+:6] = 6'b101010;
    `check(f, 4'b0101)

    // Partially oob at low side
    i = -1;
    f = 'hx;
    f[i+:2] = 2'b10;
    `check(f, 4'bxxx1)

    // Partially oob at high side
    i = 3;
    f = 'hx;
    f[i+:2] = 2'b01;
    `check(f, 4'b1xxx)

    // Fully oob at low side
    i = -2;
    f = 'hx;
    f[i+:2] = 2'b11;
    `check(f, 4'bxxxx)

    // Fully oob at high side
    i = 5;
    f = 'hx;
    f[i+:2] = 2'b11;
    `check(f, 4'bxxxx)

    // Undefined index
    i = 'hx;
    f = 'hx;
    f[i+:2] = 2'b11;
    `check(f, 4'bxxxx)
  end
  endfunction

  reg [3:0] x;

  initial begin
    x = f(1'b0);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
