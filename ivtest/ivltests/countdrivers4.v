module ts_pad (
  inout wire pad,
  input wire oe,
  input wire op
);

assign pad = oe ? op : 1'bz;

endmodule

module test();

wire bus0;
wire bus1;

reg  oe1 = 1'b0;
reg  oe2 = 1'b0;
reg  oe3 = 1'b0;
reg  oe4 = 1'b0;
reg  oe5 = 1'b0;
reg  oe6 = 1'b0;

wire op1 = 1'b0;
wire op2 = 1'b1;
wire op3 = 1'b1;
wire op4 = 1'b0;
wire op5 = 1'bx;
wire op6 = 1'bx;

ts_pad pad1(bus0, oe1, op1);
ts_pad pad2(bus1, oe2, op2);

ts_pad pad3(bus0, oe3, op3);
ts_pad pad4(bus1, oe4, op4);

bufif1(bus0, op5, oe5);
bufif1(bus1, op6, oe6);

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
  #1;
  multi = $countdrivers(bus0, forced, countD, count0, count1, countX);
  check_results(0, 0, 0, 0, 0, 0);
  multi = $countdrivers(bus1, forced, countD, count0, count1, countX);
  check_results(0, 0, 0, 0, 0, 0);
  multi = $countdrivers(pad1.pad, forced, countD, count0, count1, countX);
  check_results(0, 0, 0, 0, 0, 0);
  multi = $countdrivers(pad2.pad, forced, countD, count0, count1, countX);
  check_results(0, 0, 0, 0, 0, 0);
  $display("");

  oe1 = 1'b1;
  #1;
  multi = $countdrivers(bus0, forced, countD, count0, count1, countX);
  check_results(0, 0, 1, 1, 0, 0);
  multi = $countdrivers(bus1, forced, countD, count0, count1, countX);
  check_results(0, 0, 0, 0, 0, 0);
  multi = $countdrivers(pad1.pad, forced, countD, count0, count1, countX);
  check_results(0, 0, 1, 1, 0, 0);
  multi = $countdrivers(pad2.pad, forced, countD, count0, count1, countX);
  check_results(0, 0, 0, 0, 0, 0);
  $display("");

  oe2 = 1'b1;
  #1;
  multi = $countdrivers(bus0, forced, countD, count0, count1, countX);
  check_results(0, 0, 1, 1, 0, 0);
  multi = $countdrivers(bus1, forced, countD, count0, count1, countX);
  check_results(0, 0, 1, 0, 1, 0);
  multi = $countdrivers(pad1.pad, forced, countD, count0, count1, countX);
  check_results(0, 0, 1, 1, 0, 0);
  multi = $countdrivers(pad2.pad, forced, countD, count0, count1, countX);
  check_results(0, 0, 1, 0, 1, 0);
  $display("");

  oe3 = 1'b1;
  #1;
  multi = $countdrivers(bus0, forced, countD, count0, count1, countX);
  check_results(1, 0, 2, 1, 1, 0);
  multi = $countdrivers(bus1, forced, countD, count0, count1, countX);
  check_results(0, 0, 1, 0, 1, 0);
  multi = $countdrivers(pad1.pad, forced, countD, count0, count1, countX);
  check_results(1, 0, 2, 1, 1, 0);
  multi = $countdrivers(pad2.pad, forced, countD, count0, count1, countX);
  check_results(0, 0, 1, 0, 1, 0);
  $display("");

  oe4 = 1'b1;
  #1;
  multi = $countdrivers(bus0, forced, countD, count0, count1, countX);
  check_results(1, 0, 2, 1, 1, 0);
  multi = $countdrivers(bus1, forced, countD, count0, count1, countX);
  check_results(1, 0, 2, 1, 1, 0);
  multi = $countdrivers(pad1.pad, forced, countD, count0, count1, countX);
  check_results(1, 0, 2, 1, 1, 0);
  multi = $countdrivers(pad2.pad, forced, countD, count0, count1, countX);
  check_results(1, 0, 2, 1, 1, 0);
  $display("");

  oe5 = 1'b1;
  #1;
  multi = $countdrivers(bus0, forced, countD, count0, count1, countX);
  check_results(1, 0, 3, 1, 1, 1);
  multi = $countdrivers(bus1, forced, countD, count0, count1, countX);
  check_results(1, 0, 2, 1, 1, 0);
  multi = $countdrivers(pad1.pad, forced, countD, count0, count1, countX);
  check_results(1, 0, 3, 1, 1, 1);
  multi = $countdrivers(pad2.pad, forced, countD, count0, count1, countX);
  check_results(1, 0, 2, 1, 1, 0);
  $display("");

  oe6 = 1'b1;
  #1;
  multi = $countdrivers(bus0, forced, countD, count0, count1, countX);
  check_results(1, 0, 3, 1, 1, 1);
  multi = $countdrivers(bus1, forced, countD, count0, count1, countX);
  check_results(1, 0, 3, 1, 1, 1);
  multi = $countdrivers(pad1.pad, forced, countD, count0, count1, countX);
  check_results(1, 0, 3, 1, 1, 1);
  multi = $countdrivers(pad2.pad, forced, countD, count0, count1, countX);
  check_results(1, 0, 3, 1, 1, 1);
  $display("");

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
