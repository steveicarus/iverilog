// Check that handles of different specializations of a parameterized class
// can not be assigned to each other.

class C #(int N = 4);
endclass

module test;

  C #(8) a;
  C #(16) b;

  initial begin
    a = new;
    b = a;
    $display("FAILED");
  end

endmodule
