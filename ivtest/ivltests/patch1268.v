// This is a regression test for the bug fixed in patch tracker #1268.
module test();

reg [19:0] a[15:0];

reg [3:0] idx[3:1];

initial begin
  idx[1] = 2;
  idx[2] = 3;
  idx[3] = 4;
  a[idx[1]][idx[2]*4 +: 4] <= #(idx[3]) 4'ha;
  #4;
  $display("%h", a[2]);
  if (a[2] !== 20'hxxxxx) begin
    $display("FAILED");
    $finish;
  end
  #1;
  $display("%h", a[2]);
  if (a[2] !== 20'hxaxxx) begin
    $display("FAILED");
    $finish;
  end
  $display("PASSED");
end

endmodule
