module test;

reg i1, i2, o1, o2, o3;

event triggerA, triggerB;

integer countA = 0, countB = 0;

always_comb begin
  $display("%0t: A", $time);
  o1 = i1;
  -> triggerA;
end

always @(triggerA) begin
  countA = countA + 1;
end

always_comb begin
  $display("%0t: B", $time);
  o2 = i1;
  o3 = i2;
  -> triggerB;
end

always @(triggerB) begin
  countB = countB + 1;
end

initial begin
  #1 i1 = 1;
  #1 i2 = 1;
  #1;
  if (countA === 2 && countB === 3)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
