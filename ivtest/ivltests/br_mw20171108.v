module bug();

function signed [31:0] fpreal(
  input real in
);

real m;
real r;

begin
  m = 1 << 16;
  r = in * m;
  fpreal = $rtoi(r);
end

endfunction

function signed [31:0] fpdiv(
  input signed [31:0] a,
  input signed [31:0] b
);

reg signed [47:0] r;

begin
  r = a << 16;
  fpdiv = r / b;
end

endfunction

function signed [31:0] fpmul(
  input signed [31:0] a,
  input signed [31:0] b
);

reg signed [47:0] r;

begin
  r = a * b;
  fpmul = r >>> 16;
end

endfunction

function signed [31:0] fppow(
  input signed [31:0] a,
  input real          b
);

real ar;
real r;

begin
  ar = $itor(a) / (1 << 16);
  r = ar ** b;
  fppow = fpreal(r);
end

endfunction

wire signed [31:0] a = 1 << 16;
wire signed [31:0] b = 4 << 16;

wire signed [31:0] c = fpdiv(a, b);
wire signed [31:0] d = fppow(c, 2.0);

initial begin
  #1 $display("(%0f / %0f)**2.0 = %0f", a / 65536.0, b / 65536.0, d / 65536.0);
  if (d === 32'h0000_1000)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
