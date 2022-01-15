module test();

wire [1:0] n0 = 2'bzx;
wire [1:0] n1 = 2'b0x;
wire [1:0] n2 = 2'b1x;
wire [1:0] n3 = 2'bxx;
wire [1:0] n4 = 2'bxx;
wand [1:0] n5 = 2'bxx;
wor  [1:0] n6 = 2'bxx;

assign n4 = 2'b0x;
assign n4 = 2'b0x;
assign n4 = 2'b1x;
assign n4 = 2'b1x;
assign n4 = 2'b1x;

assign n5 = 2'b0x;
assign n5 = 2'b0x;
assign n5 = 2'b0x;
assign n5 = 2'b1x;
assign n5 = 2'b1x;

assign n6 = 2'b0x;
assign n6 = 2'b1x;
assign n6 = 2'b1x;
assign n6 = 2'bxx;
assign n6 = 2'bxx;

reg [15:0] multi;
reg [15:0] forced;
reg [15:0] countD;
reg [15:0] count0;
reg [15:0] count1;
reg [15:0] countX;

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
  multi = $countdrivers(n0[1]);
  check_results(0, -1,  -1,  -1,  -1,  -1);
  multi = $countdrivers(n0[1], forced);
  check_results(0,  0,  -1,  -1,  -1,  -1);
  multi = $countdrivers(n0[1], forced, countD);
  check_results(0,  0,   0,  -1,  -1,  -1);
  multi = $countdrivers(n0[1], forced, countD, count0);
  check_results(0,  0,   0,   0,  -1,  -1);
  multi = $countdrivers(n0[1], forced, countD, count0, count1);
  check_results(0,  0,   0,   0,   0,  -1);
  multi = $countdrivers(n0[1], forced, countD, count0, count1, countX);
  check_results(0,  0,   0,   0,   0,   0);
  force n0 = 2'bxx;
  multi = $countdrivers(n0[1], forced, countD, count0, count1, countX);
  check_results(0,  1,   0,   0,   0,   0);

  // test net driven to 0
  multi = $countdrivers(n1[1]);
  check_results(0, -1,  -1,  -1,  -1,  -1);
  multi = $countdrivers(n1[1], forced);
  check_results(0,  0,  -1,  -1,  -1,  -1);
  multi = $countdrivers(n1[1], forced, countD);
  check_results(0,  0,   1,  -1,  -1,  -1);
  multi = $countdrivers(n1[1], forced, countD, count0);
  check_results(0,  0,   1,   1,  -1,  -1);
  multi = $countdrivers(n1[1], forced, countD, count0, count1);
  check_results(0,  0,   1,   1,   0,  -1);
  multi = $countdrivers(n1[1], forced, countD, count0, count1, countX);
  check_results(0,  0,   1,   1,   0,   0);
  force n1 = 2'bxx;
  multi = $countdrivers(n1[1], forced, countD, count0, count1, countX);
  check_results(0,  1,   1,   1,   0,   0);

  // test net driven to 1
  multi = $countdrivers(n2[1]);
  check_results(0, -1,  -1,  -1,  -1,  -1);
  multi = $countdrivers(n2[1], forced);
  check_results(0,  0,  -1,  -1,  -1,  -1);
  multi = $countdrivers(n2[1], forced, countD);
  check_results(0,  0,   1,  -1,  -1,  -1);
  multi = $countdrivers(n2[1], forced, countD, count0);
  check_results(0,  0,   1,   0,  -1,  -1);
  multi = $countdrivers(n2[1], forced, countD, count0, count1);
  check_results(0,  0,   1,   0,   1,  -1);
  multi = $countdrivers(n2[1], forced, countD, count0, count1, countX);
  check_results(0,  0,   1,   0,   1,   0);
  force n2 = 2'bxx;
  multi = $countdrivers(n2[1], forced, countD, count0, count1, countX);
  check_results(0,  1,   1,   0,   1,   0);

  // test net driven to X
  multi = $countdrivers(n3[1]);
  check_results(0, -1,  -1,  -1,  -1,  -1);
  multi = $countdrivers(n3[1], forced);
  check_results(0,  0,  -1,  -1,  -1,  -1);
  multi = $countdrivers(n3[1], forced, countD);
  check_results(0,  0,   1,  -1,  -1,  -1);
  multi = $countdrivers(n3[1], forced, countD, count0);
  check_results(0,  0,   1,   0,  -1,  -1);
  multi = $countdrivers(n3[1], forced, countD, count0, count1);
  check_results(0,  0,   1,   0,   0,  -1);
  multi = $countdrivers(n3[1], forced, countD, count0, count1, countX);
  check_results(0,  0,   1,   0,   0,   1);
  force n3 = 2'bxx;
  multi = $countdrivers(n3[1], forced, countD, count0, count1, countX);
  check_results(0,  1,   1,   0,   0,   1);

  // test multi-driven net
  multi = $countdrivers(n4[1]);
  check_results(1, -1,  -1,  -1,  -1,  -1);
  multi = $countdrivers(n4[1], forced);
  check_results(1,  0,  -1,  -1,  -1,  -1);
  multi = $countdrivers(n4[1], forced, countD);
  check_results(1,  0,   6,  -1,  -1,  -1);
  multi = $countdrivers(n4[1], forced, countD, count0);
  check_results(1,  0,   6,   2,  -1,  -1);
  multi = $countdrivers(n4[1], forced, countD, count0, count1);
  check_results(1,  0,   6,   2,   3,  -1);
  multi = $countdrivers(n4[1], forced, countD, count0, count1, countX);
  check_results(1,  0,   6,   2,   3,   1);
  force n4 = 2'bxx;
  multi = $countdrivers(n4[1], forced, countD, count0, count1, countX);
  check_results(1,  1,   6,   2,   3,   1);

  // test wire and
  multi = $countdrivers(n5[1]);
  check_results(1, -1,  -1,  -1,  -1,  -1);
  multi = $countdrivers(n5[1], forced);
  check_results(1,  0,  -1,  -1,  -1,  -1);
  multi = $countdrivers(n5[1], forced, countD);
  check_results(1,  0,   6,  -1,  -1,  -1);
  multi = $countdrivers(n5[1], forced, countD, count0);
  check_results(1,  0,   6,   3,  -1,  -1);
  multi = $countdrivers(n5[1], forced, countD, count0, count1);
  check_results(1,  0,   6,   3,   2,  -1);
  multi = $countdrivers(n5[1], forced, countD, count0, count1, countX);
  check_results(1,  0,   6,   3,   2,   1);
  force n5 = 2'bxx;
  multi = $countdrivers(n5[1], forced, countD, count0, count1, countX);
  check_results(1,  1,   6,   3,   2,   1);

  // test wire or
  multi = $countdrivers(n6[1]);
  check_results(1, -1,  -1,  -1,  -1,  -1);
  multi = $countdrivers(n6[1], forced);
  check_results(1,  0,  -1,  -1,  -1,  -1);
  multi = $countdrivers(n6[1], forced, countD);
  check_results(1,  0,   6,  -1,  -1,  -1);
  multi = $countdrivers(n6[1], forced, countD, count0);
  check_results(1,  0,   6,   1,  -1,  -1);
  multi = $countdrivers(n6[1], forced, countD, count0, count1);
  check_results(1,  0,   6,   1,   2,  -1);
  multi = $countdrivers(n6[1], forced, countD, count0, count1, countX);
  check_results(1,  0,   6,   1,   2,   3);
  force n6 = 2'bxx;
  multi = $countdrivers(n6[1], forced, countD, count0, count1, countX);
  check_results(1,  1,   6,   1,   2,   3);

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
