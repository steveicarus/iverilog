// Check that a procedural copy of a whole unpacked array assigns the
// elements in left-to-right order, also when the dimensions of the source
// and the destination array are declared in opposite directions.

module test;

logic [3:0] a[1:0];
logic [3:0] b[0:1];
logic [3:0] c[1:0];
logic [3:0] d[-1:1];
logic [3:0] e[6:4];
logic [3:0] m[1:0][0:2];
logic [3:0] n[0:1][0:2];
logic [3:0] p[0:1][2:0];
real ra[2:0];
real rb[0:2];

reg failed = 0;

function automatic logic [3:0] f_first(input logic [3:0] x[0:1]);
  return x[0];
endfunction

task automatic t_first(input logic [3:0] x[0:1], output logic [3:0] y);
  y = x[0];
endtask

logic [3:0] r;

initial begin
  a[1] = 4'd1;
  a[0] = 4'd2;

  // Opposite directions: the leftmost elements are a[1] and b[0].
  b = a;
  $display("%0d %0d", b[0], b[1]);
  if (b[0] !== 4'd1 || b[1] !== 4'd2) failed = 1;

  // Same direction.
  c = a;
  $display("%0d %0d", c[1], c[0]);
  if (c[1] !== 4'd1 || c[0] !== 4'd2) failed = 1;

  // Ranges that do not start at zero.
  d[-1] = 4'd3;
  d[0] = 4'd4;
  d[1] = 4'd5;
  e = d;
  $display("%0d %0d %0d", e[6], e[5], e[4]);
  if (e[6] !== 4'd3 || e[5] !== 4'd4 || e[4] !== 4'd5) failed = 1;

  // Real valued elements.
  ra[2] = 1.5;
  ra[1] = 2.5;
  ra[0] = 3.5;
  rb = ra;
  $display("%0.1f %0.1f %0.1f", rb[0], rb[1], rb[2]);
  if (rb[0] != 1.5 || rb[1] != 2.5 || rb[2] != 3.5) failed = 1;

  // Each dimension is matched separately.
  m[1][0] = 4'd1;
  m[1][1] = 4'd2;
  m[1][2] = 4'd3;
  m[0][0] = 4'd4;
  m[0][1] = 4'd5;
  m[0][2] = 4'd6;
  n = m;
  $display("%0d %0d %0d %0d %0d %0d", n[0][0], n[0][1], n[0][2],
                                      n[1][0], n[1][1], n[1][2]);
  if (n[0][0] !== 4'd1 || n[0][1] !== 4'd2 || n[0][2] !== 4'd3 ||
      n[1][0] !== 4'd4 || n[1][1] !== 4'd5 || n[1][2] !== 4'd6) failed = 1;
  p = m;
  $display("%0d %0d %0d %0d %0d %0d", p[0][2], p[0][1], p[0][0],
                                      p[1][2], p[1][1], p[1][0]);
  if (p[0][2] !== 4'd1 || p[0][1] !== 4'd2 || p[0][0] !== 4'd3 ||
      p[1][2] !== 4'd4 || p[1][1] !== 4'd5 || p[1][0] !== 4'd6) failed = 1;

  // Array arguments of functions and tasks are copied the same way.
  r = f_first(a);
  $display("%0d", r);
  if (r !== 4'd1) failed = 1;
  t_first(a, r);
  $display("%0d", r);
  if (r !== 4'd1) failed = 1;

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
