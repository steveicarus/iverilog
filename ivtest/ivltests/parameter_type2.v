module test();

parameter signed       snv1  = 4'd1;
parameter signed [2:0] s3v1  = 4'd1;
parameter signed [3:0] s4v1  = 4'd1;
parameter signed [4:0] s5v1  = 4'd1;

parameter signed       snv15 = 4'd15;
parameter signed [2:0] s3v15 = 4'd15;
parameter signed [3:0] s4v15 = 4'd15;
parameter signed [4:0] s5v15 = 4'd15;

parameter signed       snvm1 = -4'sd1;
parameter signed [2:0] s3vm1 = -4'sd1;
parameter signed [3:0] s4vm1 = -4'sd1;
parameter signed [4:0] s5vm1 = -4'sd1;

parameter signed       snrm1 = -1.0;
parameter signed [2:0] s3rm1 = -1.0;
parameter signed [3:0] s4rm1 = -1.0;
parameter signed [4:0] s5rm1 = -1.0;

parameter              nnv1  = 4'd1;
parameter        [2:0] u3v1  = 4'd1;
parameter        [3:0] u4v1  = 4'd1;
parameter        [4:0] u5v1  = 4'd1;

parameter              nnv15 = 4'd15;
parameter        [2:0] u3v15 = 4'd15;
parameter        [3:0] u4v15 = 4'd15;
parameter        [4:0] u5v15 = 4'd15;

parameter              nnvm1 = -4'sd1;
parameter        [2:0] u3vm1 = -4'sd1;
parameter        [3:0] u4vm1 = -4'sd1;
parameter        [4:0] u5vm1 = -4'sd1;

parameter              nnrm1 = -1.0;
parameter        [2:0] u3rm1 = -1.0;
parameter        [3:0] u4rm1 = -1.0;
parameter        [4:0] u5rm1 = -1.0;

reg fail = 0;

reg match;

initial begin
  match = ($bits(snv1) == 4) && (snv1 === 1);
  $display("snv1  : %2d (%0d`b%b) %c", snv1, $bits(snv1), snv1, match ? " " : "*");
  fail = fail || !match;
  match = ($bits(s3v1) == 3) && (s3v1 === 1);
  $display("s3v1  : %2d (%0d`b%b) %c", s3v1 , $bits(s3v1), s3v1, match ? " " : "*");
  fail = fail || !match;
  match = ($bits(s4v1) == 4) && (s4v1 === 1);
  $display("s4v1  : %2d (%0d`b%b) %c", s4v1 , $bits(s4v1), s4v1, match ? " " : "*");
  fail = fail || !match;
  match = ($bits(s5v1) == 5) && (s5v1 === 1);
  $display("s5v1  : %2d (%0d`b%b) %c", s5v1 , $bits(s5v1), s5v1, match ? " " : "*");
  fail = fail || !match;

  match = ($bits(snv15) == 4) && (snv15 === -1);
  $display("snv15 : %2d (%0d`b%b) %c", snv15, $bits(snv15), snv15, match ? " " : "*");
  fail = fail || !match;
  match = ($bits(s3v15) == 3) && (s3v15 === -1);
  $display("s3v15 : %2d (%0d`b%b) %c", s3v15, $bits(s3v15), s3v15, match ? " " : "*");
  fail = fail || !match;
  match = ($bits(s4v15) == 4) && (s4v15 === -1);
  $display("s4v15 : %2d (%0d`b%b) %c", s4v15, $bits(s4v15), s4v15, match ? " " : "*");
  fail = fail || !match;
  match = ($bits(s5v15) == 5) && (s5v15 === 15);
  $display("s5v15 : %2d (%0d`b%b) %c", s5v15, $bits(s5v15), s5v15, match ? " " : "*");
  fail = fail || !match;

  match = ($bits(snvm1) == 4) && (snvm1 === -1);
  $display("snvm1 : %2d (%0d`b%b) %c", snvm1, $bits(snvm1), snvm1, match ? " " : "*");
  fail = fail || !match;
  match = ($bits(s3vm1) == 3) && (s3vm1 === -1);
  $display("s3vm1 : %2d (%0d`b%b) %c", s3vm1, $bits(s3vm1), s3vm1, match ? " " : "*");
  fail = fail || !match;
  match = ($bits(s4vm1) == 4) && (s4vm1 === -1);
  $display("s4vm1 : %2d (%0d`b%b) %c", s4vm1, $bits(s4vm1), s4vm1, match ? " " : "*");
  fail = fail || !match;
  match = ($bits(s5vm1) == 5) && (s5vm1 === -1);
  $display("s5vm1 : %2d (%0d`b%b) %c", s5vm1, $bits(s5vm1), s5vm1, match ? " " : "*");
  fail = fail || !match;

  match = (snrm1 == -1);
  $display("snrm1 : %4.1f %c", snrm1, match ? " " : "*");
  fail = fail || !match;
  match = ($bits(s3rm1) == 3) && (s3rm1 === -1);
  $display("s3rm1 : %2d (%0d`b%b) %c", s3rm1, $bits(s3rm1), s3rm1, match ? " " : "*");
  fail = fail || !match;
  match = ($bits(s4rm1) == 4) && (s4rm1 === -1);
  $display("s4rm1 : %2d (%0d`b%b) %c", s4rm1, $bits(s4rm1), s4rm1, match ? " " : "*");
  fail = fail || !match;
  match = ($bits(s5rm1) == 5) && (s5rm1 === -1);
  $display("s5rm1 : %2d (%0d`b%b) %c", s5rm1, $bits(s5rm1), s5rm1, match ? " " : "*");
  fail = fail || !match;

  match = ($bits(nnv1) == 4) && (nnv1 === 1);
  $display("nnv1  : %2d (%0d`b%b) %c", nnv1, $bits(nnv1), nnv1, match ? " " : "*");
  fail = fail || !match;
  match = ($bits(u3v1) == 3) && (u3v1 === 1);
  $display("u3v1  : %2d (%0d`b%b) %c", u3v1 , $bits(u3v1), u3v1, match ? " " : "*");
  fail = fail || !match;
  match = ($bits(u4v1) == 4) && (u4v1 === 1);
  $display("u4v1  : %2d (%0d`b%b) %c", u4v1 , $bits(u4v1), u4v1, match ? " " : "*");
  fail = fail || !match;
  match = ($bits(u5v1) == 5) && (u5v1 === 1);
  $display("u5v1  : %2d (%0d`b%b) %c", u5v1 , $bits(u5v1), u5v1, match ? " " : "*");
  fail = fail || !match;

  match = ($bits(nnv15) == 4) && (nnv15 === 15);
  $display("nnv15 : %2d (%0d`b%b) %c", nnv15, $bits(nnv15), nnv15, match ? " " : "*");
  fail = fail || !match;
  match = ($bits(u3v15) == 3) && (u3v15 === 7);
  $display("u3v15 : %2d (%0d`b%b) %c", u3v15, $bits(u3v15), u3v15, match ? " " : "*");
  fail = fail || !match;
  match = ($bits(u4v15) == 4) && (u4v15 === 15);
  $display("u4v15 : %2d (%0d`b%b) %c", u4v15, $bits(u4v15), u4v15, match ? " " : "*");
  fail = fail || !match;
  match = ($bits(u5v15) == 5) && (u5v15 === 15);
  $display("u5v15 : %2d (%0d`b%b) %c", u5v15, $bits(u5v15), u5v15, match ? " " : "*");
  fail = fail || !match;

  match = ($bits(nnvm1) == 4) && (nnvm1 === -1);
  $display("nnvm1 : %2d (%0d`b%b) %c", nnvm1, $bits(nnvm1), nnvm1, match ? " " : "*");
  fail = fail || !match;
  match = ($bits(u3vm1) == 3) && (u3vm1 === 7);
  $display("u3vm1 : %2d (%0d`b%b) %c", u3vm1, $bits(u3vm1), u3vm1, match ? " " : "*");
  fail = fail || !match;
  match = ($bits(u4vm1) == 4) && (u4vm1 === 15);
  $display("u4vm1 : %2d (%0d`b%b) %c", u4vm1, $bits(u4vm1), u4vm1, match ? " " : "*");
  fail = fail || !match;
  match = ($bits(u5vm1) == 5) && (u5vm1 === 31);
  $display("u5vm1 : %2d (%0d`b%b) %c", u5vm1, $bits(u5vm1), u5vm1, match ? " " : "*");
  fail = fail || !match;

  match = (nnrm1 == -1.0);
  $display("nnrm1 : %4.1f %c", nnrm1, match ? " " : "*");
  fail = fail || !match;
  match = ($bits(u3rm1) == 3) && (u3rm1 === 7);
  $display("u3rm1 : %2d (%0d`b%b) %c", u3rm1, $bits(u3rm1), u3rm1, match ? " " : "*");
  fail = fail || !match;
  match = ($bits(u4rm1) == 4) && (u4rm1 === 15);
  $display("u4rm1 : %2d (%0d`b%b) %c", u4rm1, $bits(u4rm1), u4rm1, match ? " " : "*");
  fail = fail || !match;
  match = ($bits(u5rm1) == 5) && (u5rm1 === 31);
  $display("u5rm1 : %2d (%0d`b%b) %c", u5rm1, $bits(u5rm1), u5rm1, match ? " " : "*");
  fail = fail || !match;

  if (fail)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
