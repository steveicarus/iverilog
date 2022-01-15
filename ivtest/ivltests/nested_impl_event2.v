module test();

reg a, b;

always @* begin  // always @(b)
  a = b;
  $display("Triggered 1 at %0t", $time);

  @*;
  $display("Triggered 2 at %0t", $time);
end

initial begin
  #10 a = 0;
  #10 a = 1;
  #10 b = 0;
  #10 b = 1;
  #10 $finish(0);
end

endmodule
