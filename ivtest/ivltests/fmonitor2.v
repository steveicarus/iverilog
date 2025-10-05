module test;

integer a, b, c;

integer mcd1a, mcd2a, mcd3a;
integer mcd1b, mcd2b, mcd3b;

integer fd;

reg [64*8:1] str;

initial begin
  a = 0;
  b = 0;
  c = 0;
  mcd1a = $fopen("log/fmonitor2.log1a"); mcd1b = $fopen("log/fmonitor2.log1b");
  mcd2a = $fopen("log/fmonitor2.log2a"); mcd2b = $fopen("log/fmonitor2.log2b");
  $fmonitor(mcd1a | mcd1b, "@%0t a = %0d", $time, a);
  $fmonitor(mcd2a | mcd2b, "@%0t b = %0d", $time, b);
  repeat (5) begin
    #1;
    a = a + 1;
    b = b + 1;
    c = c + 1;
  end
  $fclose(mcd1a);
  mcd3a = $fopen("log/fmonitor2.log3a"); mcd3b = $fopen("log/fmonitor2.log3b");
  $fmonitor(mcd3a | mcd3b, "@%0t c = %0d", $time, c);
  repeat (5) begin
    #1;
    a = a + 1;
    b = b + 1;
    c = c + 1;
  end
  $fclose(mcd2a);
  $fclose(mcd3a);

  #1;
  $fclose(mcd1b);
  $fclose(mcd2b);
  $fclose(mcd3b);

  $display("log1a:");
  fd = $fopen("log/fmonitor2.log1a", "r");
  while ($fgets(str, fd)) begin
    $write("%0s", str);
  end
  $fclose(fd);

  $display("log1b:");
  fd = $fopen("log/fmonitor2.log1b", "r");
  while ($fgets(str, fd)) begin
    $write("%0s", str);
  end
  $fclose(fd);

  $display("log2a:");
  fd = $fopen("log/fmonitor2.log2a", "r");
  while ($fgets(str, fd)) begin
    $write("%0s", str);
  end
  $fclose(fd);

  $display("log2b:");
  fd = $fopen("log/fmonitor2.log2b", "r");
  while ($fgets(str, fd)) begin
    $write("%0s", str);
  end
  $fclose(fd);

  $display("log3a:");
  fd = $fopen("log/fmonitor2.log3a", "r");
  while ($fgets(str, fd)) begin
    $write("%0s", str);
  end
  $fclose(fd);

  $display("log3b:");
  fd = $fopen("log/fmonitor2.log3b", "r");
  while ($fgets(str, fd)) begin
    $write("%0s", str);
  end
  $fclose(fd);

  $finish(0);
end

endmodule
