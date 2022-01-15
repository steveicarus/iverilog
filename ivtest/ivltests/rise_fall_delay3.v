module test();

localparam [7:0] dly1 = 1;
wire       [7:0] dly2 = 2;
reg        [7:0] dly3 = 3;

reg        en;
wire       i = 1;
wire [6:1] o;

tranif1 #(dly1, dly2)   buf1(o[1], i, en);
tranif1 #(dly2, dly1)   buf2(o[2], i, en);
tranif1 #(dly1, dly3)   buf3(o[3], i, en);
tranif1 #(dly3, dly1)   buf4(o[4], i, en);
tranif1 #(dly2, dly3+1) buf5(o[5], i, en);
tranif1 #(4,    2)      buf6(o[6], i, en);

function check(input o1, input o2, input o3, input o4, input o5, input o6);

begin
  check = (o[1] == o1) && (o[2] == o2) && (o[3] == o3)
       && (o[4] == o4) && (o[5] == o5) && (o[6] == o6);
end

endfunction

reg failed = 0;

initial begin
  #1 $monitor($time,,en,,o[1],,o[2],,o[3],,o[4],,o[5],,o[6]);

  en = 1'b1;
  #0      if (!check(1'bx, 1'bx, 1'bx, 1'bx, 1'bx, 1'bx)) failed = 1;
  #1;  #0 if (!check(1'b1, 1'bx, 1'b1, 1'bx, 1'bx, 1'bx)) failed = 1;
  #1;  #0 if (!check(1'b1, 1'b1, 1'b1, 1'bx, 1'b1, 1'bx)) failed = 1;
  #1;  #0 if (!check(1'b1, 1'b1, 1'b1, 1'b1, 1'b1, 1'bx)) failed = 1;
  #1;  #0 if (!check(1'b1, 1'b1, 1'b1, 1'b1, 1'b1, 1'b1)) failed = 1;

  en = 1'b0;
  #0      if (!check(1'b1, 1'b1, 1'b1, 1'b1, 1'b1, 1'b1)) failed = 1;
  #1;  #0 if (!check(1'b1, 1'bz, 1'b1, 1'bz, 1'b1, 1'b1)) failed = 1;
  #1;  #0 if (!check(1'bz, 1'bz, 1'b1, 1'bz, 1'b1, 1'bz)) failed = 1;
  #1;  #0 if (!check(1'bz, 1'bz, 1'bz, 1'bz, 1'b1, 1'bz)) failed = 1;
  #1;  #0 if (!check(1'bz, 1'bz, 1'bz, 1'bz, 1'bz, 1'bz)) failed = 1;

  #1;
  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
