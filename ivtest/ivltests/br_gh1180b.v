module test();

parameter [3:0] array[0:3] = '{ 4'd0, 4'd1, 4'd2, 4'd3 };

initial begin
  $display("%h %h %h %h", array[0], array[1], array[2], array[3]);
  if (array[0] === 4'h0 && array[1] === 4'h1 && array[2] === 4'h2 && array[3] === 4'h3)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
