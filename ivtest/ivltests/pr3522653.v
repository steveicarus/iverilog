module test();

initial begin
  $display("%b", 'h00000001);
  $display("%d", 'h00000001);
  $display("%b", 'hffffffff);
  $display("%d", 'hffffffff);
  $display("%b", 'sh00000001);
  $display("%d", 'sh00000001);
  $display("%b", 'shffffffff);
  $display("%d", 'shffffffff);
end

endmodule
