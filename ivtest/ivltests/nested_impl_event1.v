module test();

reg a, b, c;

always @* begin  // always @(b or c)
  a = b;
  $display("Triggered 1 at %0t", $time);

  @* a = c;  // @(c)
  $display("Triggered 2 at %0t", $time);
end

initial begin
  #10 a = 0;
  #10 a = 1;
  #10 b = 0;
  #10 b = 1;
  #10 c = 0;
  #10 c = 1;
  #10 c = 0;
  #10 $finish(0);
end

endmodule
