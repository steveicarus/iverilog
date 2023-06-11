// Check that blocking partial writes to a 2-state vector function return value
// are correctly handled.

module test;

  reg failed = 1'b0;

  `define check(val, exp) \
    if (exp !== val) begin \
      $display("FAILED. Got %b, expected %b.", val, exp); \
      failed = 1'b1; \
    end

  function bit [3:0] f;
    integer i;

    // Immediate index

    // Within bounds
    f = 'h0;
    f[2:1] = 2'b10;
    `check(f, 4'b0100)

    // Partially oob at high and low side
    f = 'h0;
    f[4:-1] = 6'b101010;
    `check(f, 4'b0101)
    // Partially oob at low side
    f = 'h0;
    f[0:-1] = 2'b10;
    `check(f, 4'b0001)
    // Partially oob at high side
    f = 'h0;
    f[4:3] = 2'b01;
    `check(f, 4'b1000)
    // Fully oob at low side
    f = 'h0;
    f[-1:-2] = 2'b11;
    `check(f, 4'b0000)
    // Fully oob at high side
    f = 'h0;
    f[6:5] = 2'b11;
    `check(f, 4'b0000)

    // Variable index

    // Within bounds
    i = 1;
    f = 'h0;
    f[i+:2] = 2'b10;
    `check(f, 4'b0100)

    // Partially oob at high and low side
    i = -1;
    f = 'h0;
    f[i+:6] = 6'b101010;
    `check(f, 4'b0101)

    // Partially oob at low side
    i = -1;
    f = 'h0;
    f[i+:2] = 2'b10;
    `check(f, 4'b0001)

    // Partially oob at high side
    i = 3;
    f = 'h0;
    f[i+:2] = 2'b01;
    `check(f, 4'b1000)

    // Fully oob at low side
    i = -2;
    f = 'h0;
    f[i+:2] = 2'b11;
    `check(f, 4'b0000)

    // Fully oob at high side
    i = 5;
    f = 'h0;
    f[i+:2] = 2'b11;
    `check(f, 4'b0000)

    // Undefined index
    i = 'hx;
    f = 'h0;
    f[i+:2] = 2'b11;
    `check(f, 4'b0000)
  endfunction

  initial begin
    bit [3:0] x;
    x = f();

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
