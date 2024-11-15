module test();

parameter [3:0][3:0] array = { 4'd0, 4'd1, 4'd2, 4'd3 };

initial begin
  $display("%h", array);
  if (array === 16'h0123)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
