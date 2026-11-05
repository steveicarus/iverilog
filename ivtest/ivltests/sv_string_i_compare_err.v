module top;
  string str1 = "ABC";
  int ival = 65;
  int res1 = str1.compare();
  int res2 = str1.compare(ival);
  int res3 = str1.compare(65);
  int res4 = str1.compare(str1, top);
  int ires1 = str1.icompare();
  int ires2 = str1.icompare(ival);
  int ires3 = str1.icompare(65);
  int ires4 = str1.icompare(str1, top);

  initial $display("FAILED: should have a compile error.");
endmodule
