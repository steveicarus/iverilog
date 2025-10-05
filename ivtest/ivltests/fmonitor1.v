module test;

integer a, b, c;

integer fd1, fd2, fd3;

reg [64*8:1] str;

initial begin
  a = 0;
  b = 0;
  c = 0;
  fd1 = $fopen("log/fmonitor1.log1", "w");
  fd2 = $fopen("log/fmonitor1.log2", "w");
  $fmonitor(fd1, "@%0t a = %0d", $time, a);
  $fmonitor(fd2, "@%0t b = %0d", $time, b);
  repeat (5) begin
    #1;
    a = a + 1;
    b = b + 1;
    c = c + 1;
  end
  $fclose(fd1);
  fd3 = $fopen("log/fmonitor1.log3", "w");
  $fmonitor(fd3, "@%0t c = %0d", $time, c);
  repeat (5) begin
    #1;
    a = a + 1;
    b = b + 1;
    c = c + 1;
  end
  $fclose(fd2);
  $fclose(fd3);

  $display("log1:");
  fd1 = $fopen("log/fmonitor1.log1", "r");
  while ($fgets(str, fd1)) begin
    $write("%0s", str);
  end
  $fclose(fd1);

  $display("log2:");
  fd2 = $fopen("log/fmonitor1.log2", "r");
  while ($fgets(str, fd2)) begin
    $write("%0s", str);
  end
  $fclose(fd2);

  $display("log3:");
  fd3 = $fopen("log/fmonitor1.log3", "r");
  while ($fgets(str, fd3)) begin
    $write("%0s", str);
  end
  $fclose(fd3);

  $finish(0);
end

endmodule
