// Check that attribute names can match visible type identifiers.

typedef int ATTR;

(* ATTR = 1 *)
module test;

  initial begin
    $display("PASSED");
  end

endmodule
