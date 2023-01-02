module test;
   reg expected = 1;
   initial begin
      $display("Const Positive %s", 1 ? "yes" : "NO");
      $display("Const Negative %s", !1 ? "yes" : "NO");
      $display("Var Positive %s", expected ? "yes" : "NO");
      $display("Var Negative %s", !expected ? "yes" : "NO");
   end
endmodule
