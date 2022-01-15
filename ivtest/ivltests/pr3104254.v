module test;

reg signed [3:0] a;
reg signed [3:0] b;
reg        [3:0] u;
reg        [3:0] r;

reg fail;

initial begin
  fail = 0;
  a = 4'b1000;
  b = 4'b0010;
  u = 4'b0001;

  r = ((a       >>> 1) | b      ) | u;
  $display("step 1 expected '0111', got '%b'", r);
  if (r !== 4'b0111) fail = 1;

  r = ((4'b1000 >>> 1) | b      ) | u;
  $display("step 2 expected '0111', got '%b'", r);
  if (r !== 4'b0111) fail = 1;

  r = ((a       >>> 1) | 4'b0010) | u;
  $display("step 3 expected '0111', got '%b'", r);
  if (r !== 4'b0111) fail = 1;

  r = ((a       >>> 1) | b      ) | 4'b0001;
  $display("step 4 expected '0111', got '%b'", r);
  if (r !== 4'b0111) fail = 1;

  r = ((4'b1000 >>> 1) | 4'b0010) | u;
  $display("step 5 expected '0111', got '%b'", r);
  if (r !== 4'b0111) fail = 1;

  r = ((a       >>> 1) | 4'b0010) | 4'b0001;
  $display("step 6 expected '0111', got '%b'", r);
  if (r !== 4'b0111) fail = 1;

  r = ((4'b1000 >>> 1) | 4'b0010) | 4'b0001;
  $display("step 7 expected '0111', got '%b'", r);
  if (r !== 4'b0111) fail = 1;

  if (fail)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
