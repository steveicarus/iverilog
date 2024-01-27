module test();

bit [7:0] a;
bit [7:0] b;
bit [7:0] c;
bit [7:0] d;

assign a[7:4] = 4'b0010;
assign b[7:6] = 2'b01;
assign b[5:4] = a[5:4];

assign c = 8'bz;

assign d = c + b - a;

initial begin
  #0 $display("%b %b %b %b", a, b, c, d);
  if (d === 8'b01000000)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
