module test;

real mema[];
real memb[];

reg failed = 0;

initial begin
  mema = new[4] ('{1.5,2.5,3.5,4.5});
  $display("%f %f %f %f", mema[0], mema[1], mema[2], mema[3]);
  memb = new[4] (mema);
  $display("%f %f %f %f", memb[0], memb[1], memb[2], memb[3]);
  if (memb[0] != 1.5 || memb[1] != 2.5 || memb[2] != 3.5 || memb[3] != 4.5) failed = 1;
  memb = new[5] (memb);
  $display("%f %f %f %f %f", memb[0], memb[1], memb[2], memb[3], memb[4]);
  if (memb[0] != 1.5 || memb[1] != 2.5 || memb[2] != 3.5 || memb[3] != 4.5 || memb[4] != 0.0) failed = 1;
  memb = new[3] (memb);
  $display("%f %f %f", memb[0], memb[1], memb[2]);
  if (memb[0] != 1.5 || memb[1] != 2.5 || memb[2] != 3.5) failed = 1;

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
