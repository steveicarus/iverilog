module test();

wire net1;
wire net2;
wire net3;
wire net4;
wire net5;

reg src1;
reg src2;
reg src3;

assign net1 = src1;
assign net2 = src2;
assign net3 = src3;

tran(net4, net1);
tran(net4, net2);
tran(net5, net3);
tran(net5, net4);

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
  src1 = 1'b0; src2 = 1'b0; src3 = 1'b0;
  #1;
  multi = $countdrivers(net1, forced, countD, count0, count1, countX);
  check_results(1, 0, 2, 2, 0, 0);
  multi = $countdrivers(net2, forced, countD, count0, count1, countX);
  check_results(1, 0, 2, 2, 0, 0);
  multi = $countdrivers(net3, forced, countD, count0, count1, countX);
  check_results(1, 0, 2, 2, 0, 0);
  multi = $countdrivers(net4, forced, countD, count0, count1, countX);
  check_results(1, 0, 3, 3, 0, 0);
  multi = $countdrivers(net5, forced, countD, count0, count1, countX);
  check_results(1, 0, 2, 2, 0, 0);
  $display("");

  src1 = 1'b1; src2 = 1'b0; src3 = 1'b0;
  #1;
  multi = $countdrivers(net1, forced, countD, count0, count1, countX);
  check_results(1, 0, 2, 0, 1, 1);
  multi = $countdrivers(net2, forced, countD, count0, count1, countX);
  check_results(1, 0, 2, 1, 0, 1);
  multi = $countdrivers(net3, forced, countD, count0, count1, countX);
  check_results(1, 0, 2, 1, 0, 1);
  multi = $countdrivers(net4, forced, countD, count0, count1, countX);
  check_results(1, 0, 3, 0, 0, 3);
  multi = $countdrivers(net5, forced, countD, count0, count1, countX);
  check_results(1, 0, 2, 0, 0, 2);
  $display("");

  src1 = 1'b1; src2 = 1'b1; src3 = 1'b0;
  #1;
  multi = $countdrivers(net1, forced, countD, count0, count1, countX);
  check_results(1, 0, 2, 0, 1, 1);
  multi = $countdrivers(net2, forced, countD, count0, count1, countX);
  check_results(1, 0, 2, 0, 1, 1);
  multi = $countdrivers(net3, forced, countD, count0, count1, countX);
  check_results(1, 0, 2, 1, 0, 1);
  multi = $countdrivers(net4, forced, countD, count0, count1, countX);
  check_results(1, 0, 3, 0, 0, 3);
  multi = $countdrivers(net5, forced, countD, count0, count1, countX);
  check_results(1, 0, 2, 0, 0, 2);
  $display("");

  src1 = 1'b1; src2 = 1'b1; src3 = 1'b1;
  #1;
  multi = $countdrivers(net1, forced, countD, count0, count1, countX);
  check_results(1, 0, 2, 0, 2, 0);
  multi = $countdrivers(net2, forced, countD, count0, count1, countX);
  check_results(1, 0, 2, 0, 2, 0);
  multi = $countdrivers(net3, forced, countD, count0, count1, countX);
  check_results(1, 0, 2, 0, 2, 0);
  multi = $countdrivers(net4, forced, countD, count0, count1, countX);
  check_results(1, 0, 3, 0, 3, 0);
  multi = $countdrivers(net5, forced, countD, count0, count1, countX);
  check_results(1, 0, 2, 0, 2, 0);
  $display("");

  src1 = 1'b1; src2 = 1'bz; src3 = 1'bz;
  #1;
  multi = $countdrivers(net1, forced, countD, count0, count1, countX);
  check_results(1, 0, 2, 0, 2, 0);
  multi = $countdrivers(net2, forced, countD, count0, count1, countX);
  check_results(0, 0, 1, 0, 1, 0);
  multi = $countdrivers(net3, forced, countD, count0, count1, countX);
  check_results(0, 0, 1, 0, 1, 0);
  multi = $countdrivers(net4, forced, countD, count0, count1, countX);
  check_results(1, 0, 3, 0, 3, 0);
  multi = $countdrivers(net5, forced, countD, count0, count1, countX);
  check_results(1, 0, 2, 0, 2, 0);
  $display("");

  src1 = 1'bz; src2 = 1'b0; src3 = 1'bz;
  #1;
  multi = $countdrivers(net1, forced, countD, count0, count1, countX);
  check_results(0, 0, 1, 1, 0, 0);
  multi = $countdrivers(net2, forced, countD, count0, count1, countX);
  check_results(1, 0, 2, 2, 0, 0);
  multi = $countdrivers(net3, forced, countD, count0, count1, countX);
  check_results(0, 0, 1, 1, 0, 0);
  multi = $countdrivers(net4, forced, countD, count0, count1, countX);
  check_results(1, 0, 3, 3, 0, 0);
  multi = $countdrivers(net5, forced, countD, count0, count1, countX);
  check_results(1, 0, 2, 2, 0, 0);
  $display("");

  src1 = 1'bz; src2 = 1'bz; src3 = 1'b1;
  #1;
  multi = $countdrivers(net1, forced, countD, count0, count1, countX);
  check_results(0, 0, 1, 0, 1, 0);
  multi = $countdrivers(net2, forced, countD, count0, count1, countX);
  check_results(0, 0, 1, 0, 1, 0);
  multi = $countdrivers(net3, forced, countD, count0, count1, countX);
  check_results(1, 0, 2, 0, 2, 0);
  multi = $countdrivers(net4, forced, countD, count0, count1, countX);
  check_results(1, 0, 3, 0, 3, 0);
  multi = $countdrivers(net5, forced, countD, count0, count1, countX);
  check_results(1, 0, 2, 0, 2, 0);
  $display("");

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
