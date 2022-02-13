// Check that modules with undefined parameters do not get picked as top-level
// modules.

module a #(
      parameter A
);
      initial $display("FAILED");
endmodule

module b #(
      parameter B = 1
);
      initial $display("PASSED");
endmodule
