// Icarus 0.7, cvs files from Feb 2, 2003
// --------------------------------------
//
//        iverilog precision.v
//    or
//        iverilog -D DUMP precision.v
//        vvp a.out
//
// Use & display of real time periods with `timescale set to 1 ns / 10 ps
//
// $simtime keeps time in 10ps increments
// $simtime cannot be displayed (yet)
// $simtime can be used in comparisons -- compared to times in 10 ps units
// $time should be $simtime-rounded-to-ns
// $time displays according to `timescale and $timeformat
// $time can be used in comparisons -- compared to times in 1 ns units
//
// Assuming that the simulation runs on units of 10ps, a clock which is set to
// change value every (15.2 ns)/2 should change every 7.6 ns, i.e. 760*10ps.
//
// The dumpfile shows a timescale of 10ps; therefore, it should show the clock
// changing every 760*10ps.  It doesn't.  The clock is changing every 700*10ps.
// The checks on the clock using $simtime below verify that the dumpfile is
// seeing what the simulation is, in fact, doing.
//

`timescale 1 ns / 10 ps

`define	PERIODI		15
`define	PERIODR		15.2

module top;

reg tick,clk, fail;
reg [31:0] ii;

`ifdef DUMP
  initial begin
    $dumpvars;
  end
`endif

initial begin
  $timeformat(-9, 2, "ns", 20);

  $display("integer & real periods: 'd%0d 'd%0d",`PERIODI,`PERIODR);
  $display("integer & real periods (15.00, 15.20): 't%0t 't%0t",`PERIODI,`PERIODR);
  $display("......... %s should be displayed as 15.20 in its timeformat.", ``PERIODR);
  $display("integer & real periods: 'b%0b 'b%0b",`PERIODI,`PERIODR);
  $display("integer & real periods: 'h%0h 'h%0h",`PERIODI,`PERIODR);

  clk = 0;
  tick = 0;
  fail = 0;
  #1;
  if($time === 1)   $display("\t$time is in ns");
  if($time === 100) $display("\t$time is in 10 ps");
   $display("\ttime (1, 1h):  'd%0d, 't%0t, 'h%0h",$time,$time,$time);
  if($simtime === 1)   $display("\t$simtime is in ns");
  if($simtime === 100) $display("\t$simtime is in 10 ps");
  $display("\tsimtime (100, 64h):  'd%0d, 't%0t, 'h%0h",$simtime,$simtime,$simtime);
  #(`PERIODI - 1);
  tick = 1;
  if($time !== 15) begin fail = 1;$display("time (15,  Fh): 'd%0d, 't%0t, 'h%0h",$time,$time,$time); end
  if($simtime !== 1500) begin fail=1; $display("simtime not 1500"); end
  #(`PERIODR);
  tick = 0;
  if($time !== 30) begin fail = 1; $display("time (30, 1Eh): 'd%0d, 't%0t, 'h%0h",$time,$time,$time); end
  if($simtime !== 3020) begin fail=1; $display("simtime not 3020"); end
  #(`PERIODR);
  tick = 1;
  if($time !== 45) begin fail = 1; $display("time (45, 2Dh): 'd%0d, 't%0t, 'h%0h",$time,$time,$time); end
  if($simtime !== 4540) begin fail=1; $display("simtime not 4540"); end
  #(`PERIODR);
  tick = 0;
  if($time !== 61) begin fail = 1; $display("time (61, 3Dh): 'd%0d, 't%0t, 'h%0h",$time,$time,$time); end
  if($simtime !== 6060) begin fail=1; $display("simtime not 6060"); end
  #(`PERIODR);
  tick = 1;
  if($time !== 76) begin fail = 1; $display("time (76, 4Ch): 'd%0d, 't%0t, 'h%0h",$time,$time,$time); end
  if($simtime !== 7580) begin fail=1; $display("simtime not 7580"); end
  #(`PERIODR);
  tick = 1;
  if($time !== 91) begin fail = 1; $display("time (91, 5Bh): 'd%0d, 't%0t, 'h%0h",$time,$time,$time); end
  if($simtime !== 9100) begin fail=1; $display("simtime not 9100"); end

  $display("\t\t**********************************************");
  if(fail) $display("\t\t****** time precision test FAILED *******");
  else     $display("\t\t****** time precision test PASSED *******");
  $display("\t\t**********************************************\n");

  $finish(0);
end

initial begin
  for(ii = 0; ii < 1524; ii = ii + 1) begin
    #(0.01);
    if(($simtime == 659) && (clk !== 0)) begin fail=1; $display("time: 659, clk wrong"); end
    if(($simtime == 701) && (clk !== 0)) begin fail=1; $display("time: 701, clk wrong"); end
    if(($simtime == 759) && (clk !== 0)) begin fail=1; $display("time: 759, clk wrong"); end
    if(($simtime == 761) && (clk !== 1)) begin fail=1; $display("time: 761, clk wrong"); end
    if(($simtime == 1399) && (clk !== 1)) begin fail=1; $display("time: 1399, clk wrong"); end
    if(($simtime == 1401) && (clk !== 1)) begin fail=1; $display("time: 1401, clk wrong"); end
    if(($simtime == 1519) && (clk !== 1)) begin fail=1; $display("time: 1519, clk wrong"); end
    if(($simtime == 1521) && (clk !== 0)) begin fail=1; $display("time: 1521, clk wrong"); end
  end
end

always  begin
  #(`PERIODR/2) clk <= ~clk;
  // clock should change as follows:
  // T (10ps)	: clk
  // 0		: 0
  // 760	: 1
  // 1520	: 0
  // 2280	: 1
  // 3040	: 0
  // etc.
end

endmodule
