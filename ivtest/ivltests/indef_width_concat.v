module top;
  parameter pval = 1;

  initial $display("Concat: %d", {pval, 2});
endmodule
