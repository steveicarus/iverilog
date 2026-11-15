// Check that a parameter value assignment is an error for a class without
// parameters and for a type that is not a class.

class C;
endclass

typedef int T;

module test;

  C #(8) a;
  T #(8) b;

  initial begin
    $display("FAILED");
  end

endmodule
