/* PR#445 */
module foo ();

initial
 if (!(1'b0))
  $display("PASSED");
 else
  $display("FAILED");

endmodule
