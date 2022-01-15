module test();

wire n0;
wire n1;
wire n2;
wire n3;
wire n4;
wand n5;
wor  n6;

assign n1 = 1'b0;
assign n2 = 1'b1;
assign n3 = 1'bx;

assign n4 = 1'b0;
assign n4 = 1'b0;
assign n4 = 1'b1;
assign n4 = 1'b1;
assign n4 = 1'b1;
assign n4 = 1'bx;

assign n5 = 1'b0;
assign n5 = 1'b0;
assign n5 = 1'b0;
assign n5 = 1'b1;
assign n5 = 1'b1;
assign n5 = 1'bx;

assign n6 = 1'b0;
assign n6 = 1'b1;
assign n6 = 1'b1;
assign n6 = 1'bx;
assign n6 = 1'bx;
assign n6 = 1'bx;

integer multi;
integer forced;
integer countD;
integer count0;
integer count1;
integer countX;

reg failed = 0;

task check_results;

input integer expected_multi;
input integer expected_forced;
input integer expected_countD;
input integer expected_count0;
input integer expected_count1;
input integer expected_countX;

begin
  $write("multi = %0d ", multi);
  if (multi !== expected_multi) failed = 1;
  if (expected_forced != -1) begin
    $write("forced = %0d ", forced);
    if (forced !== expected_forced) failed = 1;
  end
  if (expected_countD != -1) begin
    $write("countD = %0d ", countD);
    if (countD !== expected_countD) failed = 1;
  end
  if (expected_count0 != -1) begin
    $write("count0 = %0d ", count0);
    if (count0 !== expected_count0) failed = 1;
  end
  if (expected_count1 != -1) begin
    $write("count1 = %0d ", count1);
    if (count1 !== expected_count1) failed = 1;
  end
  if (expected_countX != -1) begin
    $write("countX = %0d ", countX);
    if (countX !== expected_countX) failed = 1;
  end
  $write("\n");
end

endtask

initial begin
  #0; // wait for initial values to propagate

  // test undriven net
  multi = $countdrivers(n0);
  check_results(0, -1,  -1,  -1,  -1,  -1);
  multi = $countdrivers(n0, forced);
  check_results(0,  0,  -1,  -1,  -1,  -1);
  multi = $countdrivers(n0, forced, countD);
  check_results(0,  0,   0,  -1,  -1,  -1);
  multi = $countdrivers(n0, forced, countD, count0);
  check_results(0,  0,   0,   0,  -1,  -1);
  multi = $countdrivers(n0, forced, countD, count0, count1);
  check_results(0,  0,   0,   0,   0,  -1);
  multi = $countdrivers(n0, forced, countD, count0, count1, countX);
  check_results(0,  0,   0,   0,   0,   0);
  force n0 = 1'bx;
  multi = $countdrivers(n0, forced, countD, count0, count1, countX);
  check_results(0,  1,   0,   0,   0,   0);

  // test net driven to 0
  multi = $countdrivers(n1);
  check_results(0, -1,  -1,  -1,  -1,  -1);
  multi = $countdrivers(n1, forced);
  check_results(0,  0,  -1,  -1,  -1,  -1);
  multi = $countdrivers(n1, forced, countD);
  check_results(0,  0,   1,  -1,  -1,  -1);
  multi = $countdrivers(n1, forced, countD, count0);
  check_results(0,  0,   1,   1,  -1,  -1);
  multi = $countdrivers(n1, forced, countD, count0, count1);
  check_results(0,  0,   1,   1,   0,  -1);
  multi = $countdrivers(n1, forced, countD, count0, count1, countX);
  check_results(0,  0,   1,   1,   0,   0);
  force n1 = 1'bx;
  multi = $countdrivers(n1, forced, countD, count0, count1, countX);
  check_results(0,  1,   1,   1,   0,   0);

  // test net driven to 1
  multi = $countdrivers(n2);
  check_results(0, -1,  -1,  -1,  -1,  -1);
  multi = $countdrivers(n2, forced);
  check_results(0,  0,  -1,  -1,  -1,  -1);
  multi = $countdrivers(n2, forced, countD);
  check_results(0,  0,   1,  -1,  -1,  -1);
  multi = $countdrivers(n2, forced, countD, count0);
  check_results(0,  0,   1,   0,  -1,  -1);
  multi = $countdrivers(n2, forced, countD, count0, count1);
  check_results(0,  0,   1,   0,   1,  -1);
  multi = $countdrivers(n2, forced, countD, count0, count1, countX);
  check_results(0,  0,   1,   0,   1,   0);
  force n2 = 1'bx;
  multi = $countdrivers(n2, forced, countD, count0, count1, countX);
  check_results(0,  1,   1,   0,   1,   0);

  // test net driven to X
  multi = $countdrivers(n3);
  check_results(0, -1,  -1,  -1,  -1,  -1);
  multi = $countdrivers(n3, forced);
  check_results(0,  0,  -1,  -1,  -1,  -1);
  multi = $countdrivers(n3, forced, countD);
  check_results(0,  0,   1,  -1,  -1,  -1);
  multi = $countdrivers(n3, forced, countD, count0);
  check_results(0,  0,   1,   0,  -1,  -1);
  multi = $countdrivers(n3, forced, countD, count0, count1);
  check_results(0,  0,   1,   0,   0,  -1);
  multi = $countdrivers(n3, forced, countD, count0, count1, countX);
  check_results(0,  0,   1,   0,   0,   1);
  force n3 = 1'bx;
  multi = $countdrivers(n3, forced, countD, count0, count1, countX);
  check_results(0,  1,   1,   0,   0,   1);

  // test multi-driven net
  multi = $countdrivers(n4);
  check_results(1, -1,  -1,  -1,  -1,  -1);
  multi = $countdrivers(n4, forced);
  check_results(1,  0,  -1,  -1,  -1,  -1);
  multi = $countdrivers(n4, forced, countD);
  check_results(1,  0,   6,  -1,  -1,  -1);
  multi = $countdrivers(n4, forced, countD, count0);
  check_results(1,  0,   6,   2,  -1,  -1);
  multi = $countdrivers(n4, forced, countD, count0, count1);
  check_results(1,  0,   6,   2,   3,  -1);
  multi = $countdrivers(n4, forced, countD, count0, count1, countX);
  check_results(1,  0,   6,   2,   3,   1);
  force n4 = 1'bx;
  multi = $countdrivers(n4, forced, countD, count0, count1, countX);
  check_results(1,  1,   6,   2,   3,   1);

  // test wire and
  multi = $countdrivers(n5);
  check_results(1, -1,  -1,  -1,  -1,  -1);
  multi = $countdrivers(n5, forced);
  check_results(1,  0,  -1,  -1,  -1,  -1);
  multi = $countdrivers(n5, forced, countD);
  check_results(1,  0,   6,  -1,  -1,  -1);
  multi = $countdrivers(n5, forced, countD, count0);
  check_results(1,  0,   6,   3,  -1,  -1);
  multi = $countdrivers(n5, forced, countD, count0, count1);
  check_results(1,  0,   6,   3,   2,  -1);
  multi = $countdrivers(n5, forced, countD, count0, count1, countX);
  check_results(1,  0,   6,   3,   2,   1);
  force n5 = 1'bx;
  multi = $countdrivers(n5, forced, countD, count0, count1, countX);
  check_results(1,  1,   6,   3,   2,   1);

  // test wire or
  multi = $countdrivers(n6);
  check_results(1, -1,  -1,  -1,  -1,  -1);
  multi = $countdrivers(n6, forced);
  check_results(1,  0,  -1,  -1,  -1,  -1);
  multi = $countdrivers(n6, forced, countD);
  check_results(1,  0,   6,  -1,  -1,  -1);
  multi = $countdrivers(n6, forced, countD, count0);
  check_results(1,  0,   6,   1,  -1,  -1);
  multi = $countdrivers(n6, forced, countD, count0, count1);
  check_results(1,  0,   6,   1,   2,  -1);
  multi = $countdrivers(n6, forced, countD, count0, count1, countX);
  check_results(1,  0,   6,   1,   2,   3);
  force n6 = 1'bx;
  multi = $countdrivers(n6, forced, countD, count0, count1, countX);
  check_results(1,  1,   6,   1,   2,   3);

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
