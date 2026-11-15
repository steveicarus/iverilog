// Check that overriding a parameter that is not in the parameter port list
// of a class is an error.

class C #(int N = 4);
  localparam int L = N;
endclass

module test;

  C #(.M(8)) a;
  C #(.L(8)) b;

  initial begin
    $display("FAILED");
  end

endmodule
