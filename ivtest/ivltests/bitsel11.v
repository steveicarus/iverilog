// Check that element and bit select on multi-dimensional signed packed arrays
// are unsigned.

module test;

  bit failed = 1'b0;

`define check(val, exp) do begin \
    if (val !== exp) begin \
      $display("FAILED(%0d): Expected `%b`, got `%b`.", `__LINE__, exp, val); \
      failed = 1'b1; \
    end \
  end while (0)

  reg signed [3:0][3:0] arr;
  integer idx;
  reg [7:0] x;
  reg [3:0] y;
  reg [31:0] z;

  initial begin
    // Set lowest element to all 1
    arr = 16'hffff;

    // Elements of a signed packed array are unsigned, no sign extensions
    idx = 0;
    x = arr[idx];
    `check(x, 8'h0f);

    // Out-of-bounds
    idx = -1;
    x = arr[idx];
    `check(x, 8'h0x);

    // Undefined
    idx = 'bx;
    x = arr[idx];
    `check(x, 8'h0x);

    // Bit selects on a signed packed array are unsigned, no sign extensions
    idx = 0;
    y = arr[0][idx];
    `check(y, 4'b0001);

    // Out-of-bounds
    idx = -1;
    y = arr[0][idx];
    `check(y, 4'b000x);

    // Undefined
    idx = 'bx;
    y = arr[0][idx];
    `check(y, 4'b000x);

    // The array as a primary is signed, sign extension
    z = arr;
    `check(z, 32'hffffffff);

    if (!failed) begin
      $display("PASSED");
    end
  end
endmodule
