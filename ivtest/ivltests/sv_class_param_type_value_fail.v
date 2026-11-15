// Check that a type as the value of a value parameter of a class is an
// error.

class C #(int N = 4, type T = int);
endclass

module test;

  C #(int) a;
  C #(.N(byte)) b;

  initial begin
    $display("FAILED");
  end

endmodule
