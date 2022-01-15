module m1();
parameter p = 0;
endmodule

module m2();

generate
  genvar i;
  for (i = 0; i < 2; i = i + 1) begin : Loop1
    m1 m();
    defparam m.p = 1 + i;
  end
  for (i = 2; i < 4; i = i + 1) begin : Loop2
    m1 m();
    defparam Loop2[i].m.p = 1 + i;
  end
  for (i = 4; i < 6; i = i + 1) begin : Loop3
    m1 m();
    defparam m2.Loop3[i].m.p = 1 + i;
  end
endgenerate

reg failed = 0;

initial begin
  $display("Loop1[0].m.p = %0d", Loop1[0].m.p);
  if (Loop1[0].m.p !== 1) failed = 1;
  $display("Loop1[1].m.p = %0d", Loop1[1].m.p);
  if (Loop1[1].m.p !== 2) failed = 1;
  $display("Loop2[2].m.p = %0d", Loop2[2].m.p);
  if (Loop2[2].m.p !== 3) failed = 1;
  $display("Loop2[3].m.p = %0d", Loop2[3].m.p);
  if (Loop2[3].m.p !== 4) failed = 1;
  $display("Loop3[4].m.p = %0d", Loop3[4].m.p);
  if (Loop3[4].m.p !== 5) failed = 1;
  $display("Loop3[5].m.p = %0d", Loop3[5].m.p);
  if (Loop3[5].m.p !== 6) failed = 1;

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
