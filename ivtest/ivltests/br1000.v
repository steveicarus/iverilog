module loop();

reg [3:0] a;
reg [3:0] b;
reg [3:0] c;
reg [3:0] d;

integer i;

always @* begin
  for (i = 0; i < 4; i = i + 1) begin
    b[i] = a[i];
    $display("process 1 : %0d %b", i, b);
  end
end

always @* begin
  for (i = 0; i < 4; i = i + 1) begin
    d[i] = c[i];
    $display("process 2 : %0d %b", i, d);
  end
end

initial begin
  #0;
  a = 5;
  #0;
  c = 6;
  #0;
  if ((b === 5) && (c === 6))
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
