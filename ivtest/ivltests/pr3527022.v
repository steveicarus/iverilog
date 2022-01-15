module test #(
  parameter integer i1 = 1.0, i2 = 2,
  parameter [7:0]   v1 = 3.0, v2 = 4,
  parameter real    r1 = 5.0, r2 = 6,
  parameter         u1 = 7.0, u2 = 8'd8
)(
);

parameter  integer i3 = 1.0, i4 = 2;
parameter  [7:0]   v3 = 3.0, v4 = 4;
parameter  real    r3 = 5.0, r4 = 6;
parameter          u3 = 7.0, u4 = 8'd8;

localparam integer i5 = 1.0, i6 = 2;
localparam [7:0]   v5 = 3.0, v6 = 4;
localparam real    r5 = 5.0, r6 = 6;
localparam         u5 = 7.0, u6 = 8'd8;

reg failed = 0;

initial begin
  $display("%b", i1);
  $display("%b", i2);
  $display("%b", v1);
  $display("%b", v2);
  $display("%f", r1);
  $display("%f", r2);
  $display("%f", u1);
  $display("%b", u2);

  if (i1 !== 32'd1) failed = 1;
  if (i2 !== 32'd2) failed = 1;
  if (v1 !== 8'd3)  failed = 1;
  if (v2 !== 8'd4)  failed = 1;
  if (r1 !=  5.0)   failed = 1;
  if (r2 !=  6.0)   failed = 1;
  if (u1 !=  7.0)   failed = 1;
  if (u2 !== 8'd8)  failed = 1;

  $display("%b", i3);
  $display("%b", i4);
  $display("%b", v3);
  $display("%b", v4);
  $display("%f", r3);
  $display("%f", r4);
  $display("%f", u3);
  $display("%b", u4);

  if (i3 !== 32'd1) failed = 1;
  if (i4 !== 32'd2) failed = 1;
  if (v3 !== 8'd3)  failed = 1;
  if (v4 !== 8'd4)  failed = 1;
  if (r3 !=  5.0)   failed = 1;
  if (r4 !=  6.0)   failed = 1;
  if (u3 !=  7.0)   failed = 1;
  if (u4 !== 8'd8)  failed = 1;

  $display("%b", i5);
  $display("%b", i6);
  $display("%b", v5);
  $display("%b", v6);
  $display("%f", r5);
  $display("%f", r6);
  $display("%f", u5);
  $display("%b", u6);

  if (i5 !== 32'd1) failed = 1;
  if (i6 !== 32'd2) failed = 1;
  if (v5 !== 8'd3)  failed = 1;
  if (v6 !== 8'd4)  failed = 1;
  if (r5 !=  5.0)   failed = 1;
  if (r6 !=  6.0)   failed = 1;
  if (u5 !=  7.0)   failed = 1;
  if (u6 !== 8'd8)  failed = 1;

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
