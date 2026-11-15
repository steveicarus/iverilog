// Check that more parameter values than a class has parameters is an error.

class C #(int N = 4, type T = int);
endclass

module test;

  C #(8, byte, 3) a;

  initial begin
    $display("FAILED");
  end

endmodule
