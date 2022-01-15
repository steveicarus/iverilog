module top;
  reg [1:0] lv, rv;
  real rl;
  reg res;
  string st;

  wire r1, r2, r3, r4, r5, r6, r7, r8;

  assign r1 = rl ==? rv;
  assign r2 = lv ==? rl;
  assign r3 = rl !=? rv;
  assign r4 = lv !=? rl;
  assign r1 = st ==? rv;
  assign r2 = lv ==? st;
  assign r3 = st !=? rv;
  assign r4 = lv !=? st;

  initial begin
     res = rl ==? rv;
     res = lv ==? rl;
     res = rl !=? rv;
     res = lv !=? rl;
     res = st ==? rv;
     res = lv ==? st;
     res = st !=? rv;
     res = lv !=? st;
    $display("FAILED");
  end
endmodule
