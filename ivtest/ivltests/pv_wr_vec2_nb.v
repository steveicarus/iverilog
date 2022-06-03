// Check that non-blocking partial writes to a 2-state vector are correctly
// handlded.

module test;

  reg failed = 1'b0;

  `define check(val, exp) \
    if (exp !== val) begin \
      $display("FAILED. Got %b, expected %b.", val, exp); \
      failed = 1'b1; \
    end

  bit [3:0] x;
  integer i;

  initial begin
    // Immediate index

    // Within bounds
    x = 'h0;
    x[2:1] <= 2'b10;
    #1
    `check(x, 4'b0100)

    // Partially oob at high and low side
    x = 'h0;
    x[4:-1] <= 6'b101010;
    #1
    `check(x, 4'b0101)

    // Partially oob at low side
    x = 'h0;
    x[0:-1] <= 2'b10;
    #1
    `check(x, 4'b0001)

    // Partially oob at high side
    x = 'h0;
    x[4:3] <= 2'b01;
    #1
    `check(x, 4'b1000)

    // Fully oob at low side
    x = 'h0;
    x[-1:-2] <= 2'b11;
    #1
    `check(x, 4'b0000)

    // Fully oob at high side
    x = 'h0;
    x[6:5] <= 2'b11;
    #1
    `check(x, 4'b0000)

    // Variable index

    // Within bounds
    i = 1;
    x = 'h0;
    x[i+:2] <= 2'b10;
    #1
    `check(x, 4'b0100)

    // Partially oob at high and low side
    i = -1;
    x = 'h0;
    x[i+:6] <= 6'b101010;
    #1
    `check(x, 4'b0101)

    // Partially oob at low side
    i = -1;
    x = 'h0;
    x[i+:2] <= 2'b10;
    #1
    `check(x, 4'b0001)

    // Partially oob at high side
    i = 3;
    x = 'h0;
    x[i+:2] <= 2'b01;
    #1
    `check(x, 4'b1000)

    // Fully oob at low side
    i = -2;
    x = 'h0;
    x[i+:2] <= 2'b11;
    #1
    `check(x, 4'b0000)

    // Fully oob at high side
    i = 5;
    x = 'h0;
    x[i+:2] <= 2'b11;
    #1
    `check(x, 4'b0000)

    // Undefined index
    i = 'hx;
    x = 'h0;
    x[i+:2] <= 2'b11;
    #1
    `check(x, 4'b0000)

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
