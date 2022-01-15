module test();

wire    a;

supply0 b;
supply1 c;

supply1 d;
supply0 e;

wire    f;
wire    g;

wire    h;
wire    i;

supply1 j;
supply0 k;

wire    l;

supply0 m;
supply1 n;

assign d = 1'b0;
assign e = 1'b1;

assign f = 1'b0;
assign f = 1'b1;

assign g = 1'b1;
assign g = 1'b0;

assign (strong1,strong0) h = 1'b0;
assign (  weak1,  weak0) h = 1'b1;

assign (  weak1,  weak0) i = 1'b0;
assign (strong1,strong0) i = 1'b1;

assign (supply1,supply0) j = 1'b0;
assign (supply1,supply0) k = 1'b1;

wire [1:0] A = {1'b1, a};
wire [1:0] B = {1'b1, b};
wire [1:0] C = {1'b1, c};
wire [1:0] D = {1'b1, d};
wire [1:0] E = {1'b1, e};
wire [1:0] F = {1'b1, f};
wire [1:0] G = {1'b1, g};
wire [1:0] H = {1'b1, h};
wire [1:0] I = {1'b1, i};
wire [1:0] J = {1'b1, j};
wire [1:0] K = {1'b1, k};
wire [1:0] L = {1'b1, l};
wire [1:0] M = {1'b1, m};
wire [1:0] N = {1'b1, n};

reg failed;

initial begin
  failed = 0; #1;

  $display("A = %b, expect 1z", A); if (A !== 2'b1z) failed = 1;
  $display("B = %b, expect 10", B); if (B !== 2'b10) failed = 1;
  $display("C = %b, expect 11", C); if (C !== 2'b11) failed = 1;
  $display("D = %b, expect 11", D); if (D !== 2'b11) failed = 1;
  $display("E = %b, expect 10", E); if (E !== 2'b10) failed = 1;
  $display("F = %b, expect 1x", F); if (F !== 2'b1x) failed = 1;
  $display("G = %b, expect 1x", G); if (G !== 2'b1x) failed = 1;
  $display("H = %b, expect 10", H); if (H !== 2'b10) failed = 1;
  $display("I = %b, expect 11", I); if (I !== 2'b11) failed = 1;
  $display("J = %b, expect 1x", J); if (J !== 2'b1x) failed = 1;
  $display("K = %b, expect 1x", K); if (K !== 2'b1x) failed = 1;
  force l = 1'b0; #1;
  $display("L = %b, expect 10", L); if (L !== 2'b10) failed = 1;
  force l = 1'b1; #1;
  $display("L = %b, expect 11", L); if (L !== 2'b11) failed = 1;
  force m = 1'b1; #1;
  $display("M = %b, expect 11", M); if (M !== 2'b11) failed = 1;
  force n = 1'b0; #1;
  $display("N = %b, expect 10", N); if (N !== 2'b10) failed = 1;

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
