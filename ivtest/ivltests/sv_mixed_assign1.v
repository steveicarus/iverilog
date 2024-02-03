// Check different words in an array word can be procedurally and continuously assigned.
module test();

logic [7:0] a[2:0];

assign a[0] = 8'd1;

reg failed = 0;

initial begin
  a[1] = 8'd2;
  #0 $display("%b %b %b", a[0], a[1], a[2]);
  if (a[0] !== 8'd1 || a[1] !== 8'd2 || a[2] !== 8'bx) failed = 1;
/*
 * IEEE 1800-2017 states that "A force or release statement shall not be
 * applied to a variable that is being assigned by a mixture of continuous
 * and procedural assignments.", but some other compilers allow this.  It
 * looks to be more work to detect and report it as an error than to allow
 * it.
 */
  force a[0] = 8'd3;
  #0 $display("%b %b %b", a[0], a[1], a[2]);
  if (a[0] !== 8'd3 || a[1] !== 8'd2 || a[2] !== 8'bx) failed = 1;
  force a[1] = 8'd4;
  #0 $display("%b %b %b", a[0], a[1], a[2]);
  if (a[0] !== 8'd3 || a[1] !== 8'd4 || a[2] !== 8'bx) failed = 1;
  release a[0];
  #0 $display("%b %b %b", a[0], a[1], a[2]);
  if (a[0] !== 8'd1 || a[1] !== 8'd4 || a[2] !== 8'bx) failed = 1;
  release a[1];
  #0 $display("%b %b %b", a[0], a[1], a[2]);
  if (a[0] !== 8'd1 || a[1] !== 8'd4 || a[2] !== 8'bx) failed = 1;

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
