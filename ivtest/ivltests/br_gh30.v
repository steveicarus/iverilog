// Regression test for GitHub issue #30 : failed assertion in eval_tree.cc
module bug();

wire [3:0] y;

assign y = 1 * (1 ? (1.0 * 0) : 1);

initial begin
  if (y === 4'd0)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
