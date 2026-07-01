// Regression: comparing a `string` parameter to a string literal of a different
// length in a generate condition must constant-fold, not assert in eval_eqeq_
// ("failed assertion lv.len() == rv.len()").

module main #(parameter string Target = "A");
   if (Target == "AA")
     initial $display("FAILED");
   else
     initial $display("PASSED");
endmodule
