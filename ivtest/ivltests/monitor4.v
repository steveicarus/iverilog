// Test that $monitor correctly cancels a preceding $monitor.

module test;

integer a, b;

initial begin
  a = 0;
  b = 0;
  $monitor("@%0t a = %0d", $time, a);
  repeat (5) begin
    #1;
    a = a + 1;
    b = b + 1;
  end
  $monitor("@%0t b = %0d", $time, b);
  repeat (5) begin
    #1;
    a = a + 1;
    b = b + 1;
  end
  $monitoroff;
  repeat (5) begin
    #1;
    a = a + 1;
    b = b + 1;
  end
  #0;
  $monitoron;
  repeat (5) begin
    #1;
    a = a + 1;
    b = b + 1;
  end
  $finish(0);
end

endmodule
