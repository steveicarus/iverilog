module test;

string mema[];
string memb[];

reg failed = 0;

initial begin
  mema = new[4] ('{"A","B","C","D"});
  $display("%s %s %s %s", mema[0], mema[1], mema[2], mema[3]);
  memb = new[4] (mema);
  $display("%s %s %s %s", memb[0], memb[1], memb[2], memb[3]);
  if (memb[0] != "A" || memb[1] != "B" || memb[2] != "C" || memb[3] != "D") failed = 1;
  memb = new[5] (memb);
  $display("%s %s %s %s %s", memb[0], memb[1], memb[2], memb[3], memb[4]);
  if (memb[0] != "A" || memb[1] != "B" || memb[2] != "C" || memb[3] != "D" || memb[4] != "") failed = 1;
  memb = new[3] (memb);
  $display("%s %s %s", memb[0], memb[1], memb[2]);
  if (memb[0] != "A" || memb[1] != "B" || memb[2] != "C") failed = 1;

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
