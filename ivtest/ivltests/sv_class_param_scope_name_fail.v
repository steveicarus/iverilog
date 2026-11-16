// Check that the errors for names that are not in a class scope are reported:
// a name the class does not have, and a scope that is not a class.

class C #(int N = 4);
endclass

module test;

  typedef int myint;

  localparam int A = C#(8)::MISSING;
  localparam int B = myint::N;

  initial begin
    $display("FAILED");
  end

endmodule
