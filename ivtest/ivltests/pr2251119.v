module bug();

real	t1;
integer	t2;

initial begin
  t1 = 2000;
  t2 = 1000;
  if (0.55*t1 < t2)
    $display("FAILED: %0d < %0d true branch taken", 0.55*t1, t2);
  else
    $display("PASSED: %0d < %0d false branch taken", 0.55*t1, t2);
end

endmodule
