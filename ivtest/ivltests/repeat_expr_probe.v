module top;
  reg [2:0] delay;
  integer lp;

  initial begin
    lp = 0;
    delay = 3'b000;
    // The delay+4 expression was failing because probe_expr_width() was
    // not called before elab_and_eval() with an expression width of -1.
    repeat (delay+4) lp = lp + 1;
    if (lp != 4) $display("FAILED");
    else $display("PASSED");
  end
endmodule
