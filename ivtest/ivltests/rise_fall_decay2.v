module test();

localparam [7:0] dly1 = 4;
wire       [7:0] dly2 = 3;
reg        [7:0] dly3 = 2;

reg        i;
wire [6:1] o;

nmos #(dly1,   dly2,   dly3)   buf1(o[1], i, 1'b1);
nmos #(dly2,   dly3,   dly1)   buf2(o[2], i, 1'b1);
nmos #(dly3,   dly1,   dly2)   buf3(o[3], i, 1'b1);
nmos #(dly2-1, dly2-2, dly2-3) buf4(o[4], i, 1'b1);
nmos #(dly3+1, dly3+2, dly3+3) buf5(o[5], i, 1'b1);
nmos #(4,      3,      2)      buf6(o[6], i, 1'b1);

function check(input o1, input o2, input o3, input o4, input o5, input o6);

begin
  check = (o[1] == o1) && (o[2] == o2) && (o[3] == o3)
       && (o[4] == o4) && (o[5] == o5) && (o[6] == o6);
end

endfunction

reg failed = 0;

initial begin
  #1 $monitor($time,,i,,o[1],,o[2],,o[3],,o[4],,o[5],,o[6]);

  i = 1'b1;
  #0      if (!check(1'bx, 1'bx, 1'bx, 1'bx, 1'bx, 1'bx)) failed = 1;
  #1;  #0 if (!check(1'bx, 1'bx, 1'bx, 1'bx, 1'bx, 1'bx)) failed = 1;
  #1;  #0 if (!check(1'bx, 1'bx, 1'b1, 1'b1, 1'bx, 1'bx)) failed = 1;
  #1;  #0 if (!check(1'bx, 1'b1, 1'b1, 1'b1, 1'b1, 1'bx)) failed = 1;
  #1;  #0 if (!check(1'b1, 1'b1, 1'b1, 1'b1, 1'b1, 1'b1)) failed = 1;

  i = 1'b0;
  #0      if (!check(1'b1, 1'b1, 1'b1, 1'b1, 1'b1, 1'b1)) failed = 1;
  #1;  #0 if (!check(1'b1, 1'b1, 1'b1, 1'b0, 1'b1, 1'b1)) failed = 1;
  #1;  #0 if (!check(1'b1, 1'b0, 1'b1, 1'b0, 1'b1, 1'b1)) failed = 1;
  #1;  #0 if (!check(1'b0, 1'b0, 1'b1, 1'b0, 1'b1, 1'b0)) failed = 1;
  #1;  #0 if (!check(1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0)) failed = 1;

  i = 1'bx;
  #0      if (!check(1'b0, 1'b0, 1'b0, 1'bx, 1'b0, 1'b0)) failed = 1;
  #1;  #0 if (!check(1'b0, 1'b0, 1'b0, 1'bx, 1'b0, 1'b0)) failed = 1;
  #1;  #0 if (!check(1'bx, 1'bx, 1'bx, 1'bx, 1'b0, 1'bx)) failed = 1;
  #1;  #0 if (!check(1'bx, 1'bx, 1'bx, 1'bx, 1'bx, 1'bx)) failed = 1;

  i = 1'bz;
  #0      if (!check(1'bx, 1'bx, 1'bx, 1'bz, 1'bx, 1'bx)) failed = 1;
  #1;  #0 if (!check(1'bx, 1'bx, 1'bx, 1'bz, 1'bx, 1'bx)) failed = 1;
  #1;  #0 if (!check(1'bz, 1'bx, 1'bx, 1'bx, 1'bx, 1'bx)) failed = 1;
  #1;  #0 if (!check(1'bz, 1'bx, 1'bx, 1'bz, 1'bx, 1'bz)) failed = 1;
  #1;  #0 if (!check(1'bz, 1'bz, 1'bz, 1'bz, 1'bx, 1'bz)) failed = 1;
  #1;  #0 if (!check(1'bz, 1'bz, 1'bz, 1'bz, 1'bz, 1'bz)) failed = 1;

  #1;
  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
