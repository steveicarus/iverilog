module cast_large_real;

reg        [63:0] u64;
reg signed [63:0] i64;

reg        [64:0] u65;
reg signed [64:0] i65;

real              r;

reg               fail;

initial begin
  fail = 0;

  u64 = {1'b1, 63'd0};
  r = u64;
  $display("Convert u64 to real");
  $display("Expect : %0f", 2.0**63);
  $display("Got    : %0f", r);
  if (r != 2.0**63) fail = 1;
  u64 = r;
  $display("Convert real to u64");
  $display("Expect : %0d", {1'b1, 63'd0});
  $display("Got    : %0d", u64);
  if (u64 != {1'b1, 63'd0}) fail = 1;

  i64 = {1'b1, 63'd0};
  r = i64;
  $display("Convert i64 to real");
  $display("Expect : %0f", -(2.0**63));
  $display("Got    : %0f", r);
  if (r != -(2.0**63)) fail = 1;
  i64 = r;
  $display("Convert real to i64");
  $display("Expect : %0d", $signed({1'b1, 63'd0}));
  $display("Got    : %0d", i64);
  if (i64 != {1'b1, 63'd0}) fail = 1;

  u65 = {1'b1, 64'd0};
  r = u65;
  $display("Convert u65 to real");
  $display("Expect : %0f", 2.0**64);
  $display("Got    : %0f", r);
  if (r != 2.0**64) fail = 1;
  u65 = r;
  $display("Convert real to u65");
  $display("Expect : %0d", {1'b1, 64'd0});
  $display("Got    : %0d", u65);
  if (u65 != {1'b1, 64'd0}) fail = 1;

  i65 = {1'b1, 64'd0};
  r = i65;
  $display("Convert i65 to real");
  $display("Expect : %0f", -(2.0**64));
  $display("Got    : %0f", r);
  if (r != -(2.0**64)) fail = 1;
  i65 = r;
  $display("Convert real to i65");
  $display("Expect : %0d", $signed({1'b1, 64'd0}));
  $display("Got    : %0d", i65);
  if (i65 != {1'b1, 64'd0}) fail = 1;

  if (fail)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
