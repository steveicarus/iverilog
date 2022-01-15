module test;

bit [7:0] mema[];
bit [7:0] memb[];

reg failed = 0;

initial begin
  mema = new[4] ('{8'd1,8'd2,8'd3,8'd4});
  $display("%x %x %x %x", mema[0], mema[1], mema[2], mema[3]);
  memb = new[4] (mema);
  $display("%x %x %x %x", memb[0], memb[1], memb[2], memb[3]);
  if (memb[0] !== 8'd1 || memb[1] !== 8'd2 || memb[2] !== 8'd3 || memb[3] !== 8'd4) failed = 1;
  memb = new[5] (memb);
  $display("%x %x %x %x %x", memb[0], memb[1], memb[2], memb[3], memb[4]);
  if (memb[0] !== 8'd1 || memb[1] !== 8'd2 || memb[2] !== 8'd3 || memb[3] !== 8'd4 || memb[4] !== 8'b0) failed = 1;
  memb = new[3] (memb);
  $display("%x %x %x", memb[0], memb[1], memb[2]);
  if (memb[0] !== 8'd1 || memb[1] !== 8'd2 || memb[2] !== 8'd3) failed = 1;

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
