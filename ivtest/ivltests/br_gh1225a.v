module test;
  genvar gv1;

  // initialization and increment genvars do not match
  for (genvar gv2 = -1; -1; gv1 = -1);
endmodule
