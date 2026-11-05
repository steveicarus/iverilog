module top;
  parameter string str1 = "ABC";
  parameter int ival = 65;
  parameter int res1 = str1.compare();
  parameter int res2 = str1.compare(ival);
  parameter int res3 = str1.compare(str1, top);
  parameter int ires1 = str1.icompare();
  parameter int ires2 = str1.icompare(ival);
  parameter int ires3 = str1.icompare(str1, top);

  initial $display("FAILED: should have a compile error.");
endmodule
