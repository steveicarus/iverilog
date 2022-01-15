module switch_primitives();

wire a;
wire b0;
wire b1;
wire m0;
wire m1;
wire t0;
wire t1;
reg  in;
reg  en;

bufif0(b0, a, en);
bufif1(b1, a, en);

pmos(m0, a, en);
nmos(m1, a, en);

tranif0(t0, a, en);
tranif1(a, t1, en);

assign a = in;

initial begin
  $monitor("%b %b  %b %b  %b %b  %b %b  %v %v  %v %v  %v %v",
           en, a, b0, b1, m0, m1, t0, t1,
                  b0, b1, m0, m1, t0, t1);
  #1 $display("------------------");
  #1 en = 1'b0; in = 1'b0;
  #1 en = 1'b0; in = 1'b1;
  #1 en = 1'b0; in = 1'bx;
  #1 en = 1'b0; in = 1'bz;
  #1 $display("------------------");
  #1 en = 1'b1; in = 1'b0;
  #1 en = 1'b1; in = 1'b1;
  #1 en = 1'b1; in = 1'bx;
  #1 en = 1'b1; in = 1'bz;
  #1 $display("------------------");
  #1 en = 1'bx; in = 1'b0;
  #1 en = 1'bx; in = 1'b1;
  #1 en = 1'bx; in = 1'bx;
  #1 en = 1'bx; in = 1'bz;
  #1 $display("------------------");
  #1 en = 1'bz; in = 1'b0;
  #1 en = 1'bz; in = 1'b1;
  #1 en = 1'bz; in = 1'bx;
  #1 en = 1'bz; in = 1'bz;
end

endmodule
