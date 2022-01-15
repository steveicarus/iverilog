module bug();

reg clock = 0;

always begin
  #1 clock = 1;
  #1 clock = 0;
end

integer count = 0;

initial begin:counter
  forever begin
    repeat (2) @(posedge clock);
    count = count + 1;
    $display(count);
  end
end

initial begin
  repeat (5) @(posedge clock);
  disable counter;
  repeat (4) @(posedge clock);
  if (count === 2)
    $display("PASSED");
  else
    $display("FAILED");
  $finish;
end

endmodule
