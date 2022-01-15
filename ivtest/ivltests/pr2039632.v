module top;
    // Check conditional generate.
  generate if (foo != "bar") initial $display("In conditional");
  endgenerate

    // Check loop generate
  generate for (lp = foo; lp < 1; lp = lp + 1) initial $display("Loop %f", lp);
  endgenerate

  generate for (lp = 0; lp < foo; lp = lp + 1) initial $display("Loop %f", lp);
  endgenerate

  generate for (lp = 5; lp < 6; lp = lp + foo) initial $display("Loop %f", lp);
  endgenerate

    // Check case generate.
  generate case (foo)
      1: initial $display("One case");
      default: initial $display("Default case");
    endcase
  endgenerate
endmodule
