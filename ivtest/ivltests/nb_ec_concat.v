// Check that non-blocking event control assignments to concatanations work as
// expected.

module test;
  reg failed = 1'b0;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED. %s: expected %b, got %b.", `"val`", exp, val); \
      failed = 1'b1; \
    end

  reg [3:0] x;
  reg [3:0] y;
  reg [3:0] z[1:0];

  integer i = 0;
  event e;

  initial begin
    // Test all of
    //  * vector
    //  * vector part select
    //  * array element
    //  * array element part

    // Immediate index
    {z[1][1:0],z[0],y[1:0],x} <= @e 12'h5a5;
    #1
    // Assignment must not occur until the event is triggered
    `check(z[1], 4'bxxxx)
    `check(z[0], 4'bxxxx)
    `check(y, 4'bxxxx)
    `check(x, 4'bxxxx)

    ->e;
    `check(z[1], 4'bxx01);
    `check(z[0], 4'b0110);
    `check(y, 4'bxx10);
    `check(x, 4'b0101);

    x = 4'hx;
    y = 4'hx;
    z[0] = 4'hx;
    z[1] = 4'hx;

    // Immediate index reverse order
    {x,y[1:0],z[0],z[1][1:0]} <= @e 12'ha5a;
    #1
    `check(z[1], 4'bxxxx)
    `check(z[0], 4'bxxxx)
    `check(y, 4'bxxxx)
    `check(x, 4'bxxxx)

    ->e;
    `check(z[1], 4'bxx10);
    `check(z[0], 4'b0110);
    `check(y, 4'bxx01);
    `check(x, 4'b1010);

    x = 4'hx;
    y = 4'hx;
    z[0] = 4'hx;
    z[1] = 4'hx;

    // Variable index
    {z[i+1][i+:2],z[i],y[i+:2],x} <= @e 12'h5a5;
    #1
    `check(z[1], 4'bxxxx)
    `check(z[0], 4'bxxxx)
    `check(y, 4'bxxxx)
    `check(x, 4'bxxxx)

    ->e;
    `check(z[1], 4'bxx01);
    `check(z[0], 4'b0110);
    `check(y, 4'bxx10);
    `check(x, 4'b0101);

    x = 4'hx;
    y = 4'hx;
    z[0] = 4'hx;
    z[1] = 4'hx;

    // Variable index reverse order
    {x,y[i+:2],z[i],z[i+1][i+:2]} <= @e 12'ha5a;
    #1
    `check(z[1], 4'bxxxx)
    `check(z[0], 4'bxxxx)
    `check(y, 4'bxxxx)
    `check(x, 4'bxxxx)

    ->e;
    `check(z[1], 4'bxx10);
    `check(z[0], 4'b0110);
    `check(y, 4'bxx01);
    `check(x, 4'b1010);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
