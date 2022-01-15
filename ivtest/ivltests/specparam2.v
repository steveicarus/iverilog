module test();

specparam       sp1 = 1;
specparam       sp2 = 2;
specparam [3:0] sp3 = 4'b0101;

localparam      lp1 = {sp2{1'b1}};
localparam      lp2 = sp3[sp1 +: sp2];

reg pass = 1;

initial begin
  $display("%b", lp1);
  if (($bits(lp1) != 2) || (lp1 !== 2'b11)) pass = 0;
  $display("%b", lp2);
  if (($bits(lp2) != 2) || (lp2 !== 2'b10)) pass = 0;

  if (pass)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
