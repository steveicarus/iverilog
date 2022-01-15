module main;

reg[63:0]  period;

initial begin
  if (period !== 'hx) $display ("init wrong");
  if (period === 'hx)
    $display ("init right");
  else
    $display ("init wrong 2: %h", period);
end

always @ (period) begin
//  if (period == 10) $display("%t hurrah!",$time);
  if (period !== 1'bx)
    $display ("right %t %d", $time,period);
  else
    $display("wrong %t %d",$time,period);
end

initial begin
  #10 period = $time;
  #30 $display("bye.");
  $finish(0);
end

endmodule
