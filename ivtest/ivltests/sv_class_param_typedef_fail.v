// A typedef name is not a class name, so it can not take a parameter value
// assignment, even if the typedef names a specialization of a class.

class C #(int N = 4);
  logic [N-1:0] vec;
endclass

typedef C #(8) c8_t;
typedef C c_t;

package p;
  class D #(int N = 4);
  endclass
endpackage

// A typedef named like a class is still a typedef.
typedef p::D #(5) D;

module test;

  c8_t a;
  c8_t #(4) b;
  c_t #(2) c;
  D #(7) d;

  initial begin
    $display("FAILED");
  end

endmodule
