module test;
  // genvar does not increment.
  for (genvar gv1 = -1; gv1 <= 2; gv1 = -1);

  // genvar duplicates.
  for (genvar gv2 = 0; gv2 >= 0; gv2 = gv2 === 1 ? gv2 - 1 : gv2 + 1);

  // undefined value in initialization
  for (genvar gv3 = 1'bx; gv3 <= 1; gv3 = gv3 + 1);

  // undefined value in increment
  for (genvar gv4 = 0; gv4 <= 1; gv4 = gv4 + 1'bx);

  // undefined value in test condition
  for (genvar gv5 = 0; 1'bx; gv5 = gv5 + 1);

  // genvar used in RHS of initialization
  for (genvar gv6 = gv6 + 1; gv <= 1; gv6 = gv6 + 1);

  // loop forever
  for (genvar gv7 = 0; 1'b1; gv7 = gv7 + 1);

  // Almost forever
  for (genvar gv8 = 0; gv8 >= 0; gv8 = gv8 + 1);
endmodule
