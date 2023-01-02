// Check that null-bytes are handled consistently between string literals,
// number literals and signals of all kinds, especially when formatting as a
// string.

module test;

  reg failed = 1'b0;

  `define check(val, exp) \
    if (val != exp) begin \
      $display("FAILED(%0d): Expected '%0s', got '%0s'.", `__LINE__, exp, val); \
      failed = 1'b1; \
    end

  reg [255:0] s;
  reg [31:0] x;
  reg [31:0] y[1:0];
  wire [31:0] z;
  wire [31:0] w;

  assign z = "\000a\000b";
  assign w = 32'h00610062;

  initial begin
    $sformat(s, ":%x:%0x:%s:%0s:", "\000a\000b", "\000a\000b", "\000a\000b", "\000a\000b");
    `check(s, ":00610062:610062: a b:a b:")
    $sformat(s, ":%x:%0x:%s:%0s:", 32'h00610062, 32'h00610062, 32'h00610062, 32'h00610062);
    `check(s, ":00610062:610062: a b:a b:")

    x = "\000a\000b";
    $sformat(s, ":%x:%0x:%s:%0s:", x, x, x, x);
    `check(s, ":00610062:610062: a b:a b:")

    x = 32'h00610062;
    $sformat(s, ":%x:%0x:%s:%0s:", x, x, x, x);
    `check(s, ":00610062:610062: a b:a b:")

    y[0] = "\000a\000b";
    $sformat(s, ":%x:%0x:%s:%0s:", y[0], y[0], y[0], y[0]);
    `check(s, ":00610062:610062: a b:a b:")

    y[1] = 32'h00610062;
    $sformat(s, ":%x:%0x:%s:%0s:", y[1], y[1], y[1], y[1]);
    `check(s, ":00610062:610062: a b:a b:")

    $sformat(s, ":%x:%0x:%s:%0s:", z, z, z, z);
    `check(s, ":00610062:610062: a b:a b:")

    $sformat(s, ":%x:%0x:%s:%0s:", w, w, w, w);
    `check(s, ":00610062:610062: a b:a b:")


    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
