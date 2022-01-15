module test;

event i1, i2, i3;

integer countA, countB;

always @(i1 or i2) begin
  $display("%0t: A", $time);
  countA = countA + 1;
end

always @(i2 or i3) begin
  $display("%0t: B", $time);
  countB = countB + 1;
end

initial begin
  countA = 0;
  countB = 0;
  #1 ->i1;
  #1 ->i2;
  #1 ->i3;
  #1;
  if (countA === 2 && countB === 2)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
