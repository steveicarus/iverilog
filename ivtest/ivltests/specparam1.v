module test();

specparam       sp1 = 2'd1;
specparam [1:0] sp2 = 2;
specparam       sp3 = 3.5;

specify
  specparam       sp4 = sp1;
  specparam [1:0] sp5 = sp2 + 1;
  specparam       sp6 = sp3 + 1.0;
endspecify

reg pass = 1;

initial begin
  $display("%b", sp1);
  if (($bits(sp1) != 2) || (sp1 !== 2'd1)) pass = 0;
  $display("%b", sp2);
  if (($bits(sp2) != 2) || (sp2 !== 2'd2)) pass = 0;
  $display("%f", sp3);
  if (sp3 != 3.5) pass = 0;

  $display("%b", sp4);
  if (($bits(sp4) != 2) || (sp4 !== 2'd1)) pass = 0;
  $display("%b", sp5);
  if (($bits(sp5) != 2) || (sp5 !== 2'd3)) pass = 0;
  $display("%f", sp6);
  if (sp6 != 4.5) pass = 0;

  if (pass)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
